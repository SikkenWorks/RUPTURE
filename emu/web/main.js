// The page: the front panel (drawn by the same Rust code as the native window, in its own wasm
// instance), the pointer and keyboard input, the firmware and clock pickers, and the audio graph
// that runs the emulator. Every firmware boots from its whole update package (.fwsc), unpacked
// in the wasm: a catalog entry's fetched from its author's site, or one opened from this
// computer (M-VAVE's own: their server does not let other pages fetch it).
import { fetchPackage } from "./fwsc.js";
import { isFirmwareLocked, verifyPinnedPackage, assertAllowedSave, assertFirmwareImportAllowed } from "./firmware-policy.js";
import { SaveStore, packageHash, changedSectors, packSectors, exportSave, importSave } from "./persistence.js";
import { tryOutOf, loadTryOut } from "./tryout.js";
import { createPanelScheduler } from "./render-scheduler.js";
import { DIAGNOSTIC_SECONDS, createAccuracySession, observeAccuracyFrame, summarizeAccuracy, accuracyReport, diagnosticSummaryText } from "./accuracy.js";

const CLOCKS = [60, 80, 100, 120, 160, 200, 240];
const LOCKED_FIRMWARE = isFirmwareLocked(document.documentElement);
// The device this page is for: the FM-1 (index.html) or the SMK-37 (smk37/index.html, <html
// data-device="smk37">): its catalog, words and panel. A package says which board it runs on, and
// the panel follows it (an SMK-37 package opened on the FM-1 page gets the SMK-37's).
const DEVICE = document.documentElement.dataset.device === "smk37" ? "smk37" : "fm1";
const DEV = DEVICE === "smk37"
  ? { product: "M-VAVE SMK-37", board: 1, key: "smk37.", stock: "SMK-37 Pro_016.fwsc", stockFile: /^SMK-37 (Pro|Elite)_\d+( \(\d+\))?\.fwsc$/i,
      other: { name: "FM-1", href: "../" } }
  : { product: "M-VAVE FM-1", board: 0, key: "fm1.", stock: "FM-1.fwsc", stockFile: /^FM-1( \(\d+\))?\.fwsc$/i,
      other: { name: "SMK-37", href: "smk37/" } };
const params = new URLSearchParams(location.search);
const stored = (k) => { try { return localStorage.getItem(k); } catch { return null; } };
const store = (k, v) => { try { localStorage.setItem(k, v); } catch { /* private mode */ } };

const catalog = await (await fetch("firmwares.json")).json();
const byId = new Map(catalog.firmwares.map((f) => [f.id, f]));
// the FM-1 Firmware Hub's ids too, where they differ (its cards link here with ?fw=<its id>)
for (const f of catalog.firmwares) if (f.hub && !byId.has(f.hub)) byId.set(f.hub, f);
// ?embed=1: only the device and its controls, for a window on another site (the inline script in
// <head> set data-embed before the first paint; style.css hides the rest). It does not change the
// firmware this browser starts with on the full page.
const EMBED = document.documentElement.hasAttribute("data-embed");
let fw = (LOCKED_FIRMWARE ? byId.get(catalog.default) : (byId.get(params.get("fw")) || byId.get(stored(DEV.key + "fw")))) || byId.get(catalog.default) || catalog.firmwares[0];
// ?flash= / ?script=: a try-out of that firmware (web/tryout.js), from this site's own files: a session of its
// own (this browser's saved flash neither loaded nor saved), started at once where the browser lets it play
let tryOut = null, tryOutError = null;
try {
  tryOut = LOCKED_FIRMWARE ? null : tryOutOf(params, location.href);
  if (tryOut) Object.assign(tryOut, { fw, files: null, told: null });
} catch (error) { tryOutError = `try-out: ${error.message}`; }
// how the try-out goes, to the page that embeds this one when it is on this origin (never elsewhere)
function tell(state, text = "") {
  if (typeof window === "undefined" || window.parent === window) return;
  try { window.parent.postMessage({ source: "fm1-emu", type: "tryout", state, text }, location.origin); } catch { /* not ours */ }
}
function endTryOut() {
  if (!tryOut) return;
  tryOut = null;
  const q = new URLSearchParams(location.search);
  q.delete("flash");
  q.delete("script");
  history.replaceState(null, "", `?${q}`);
}
let mhz = Number(params.get("mhz") || stored("fm1.mhz") || 100);
let accurate = (params.get("mode") || stored("fm1.mode")) === "accurate";
// the audio buffer: "interactive" (small: keys answer quickly) unless this device needed a larger
// one ("playback", offered when the sound drops out with time to spare; ?latency= overrides)
let latency = params.get("latency") || stored("fm1.latency") || "interactive";
let performanceMhz = Number(stored("fm1.performance.mhz") || (!accurate ? mhz : 100));
// The emulation queue trades input latency for available worklet headroom.
// Keep the V2.2 stable default until a user explicitly chooses a shorter queue.
let audioAhead = Number(params.get("ahead") || stored("fm1.ahead") || 1024);
if (![256, 512, 1024].includes(audioAhead)) audioAhead = 1024;

const canvas = document.getElementById("panel");
const ctx = canvas.getContext("2d", { alpha: false, desynchronized: true });
const status = document.getElementById("status");
const startBtn = document.getElementById("start");
const fwSel = document.getElementById("fw");
const mhzSel = document.getElementById("mhz");
const modeSel = document.getElementById("mode");
const audioAheadSel = document.getElementById("audio-ahead");
const about = document.getElementById("about");
const title = document.getElementById("title");
const openInput = document.getElementById("open");
const bankInput = document.getElementById("bank");
const bankName = document.getElementById("bankname");
const saveStatus = document.getElementById("save-status");
const toast = document.getElementById("toast");
let toastTimer = 0;
function announce(message, failed = false) {
  clearTimeout(toastTimer);
  toast.textContent = message;
  toast.dataset.error = String(failed);
  toast.hidden = false;
  toastTimer = setTimeout(() => { toast.hidden = true; }, 3300);
}
// V2.1 Accuracy Lab is opt-in: measuring callbacks adds a small amount of work.
// It never captures played notes or raw audio. Stop on every firmware reboot.
const diagDialog = document.getElementById("diagnostics-dialog");
const diagStatus = document.getElementById("diagnostics-status");
const diagResult = document.getElementById("diagnostics-result");
const diagRun = document.getElementById("diagnostics-run");
const diagExport = document.getElementById("diagnostics-export");
let diagnosis = null, diagnosisSeq = 0, diagnosisActive = false;
function stopDiagnostics(reason = null) {
  if (!diagnosisActive) return;
  diagnosisActive = false;
  node?.port.postMessage({ type: "monitor", enabled: false });
  diagRun.disabled = false;
  diagExport.disabled = !diagnosis?.samples.length;
  if (diagnosis?.samples.length) diagResult.textContent = diagnosticSummaryText(diagnosis);
  diagStatus.textContent = reason || `Test completed. ${diagnosis?.samples.length || 0} measurement intervals collected.`;
}
document.getElementById("diagnostics-open").addEventListener("click", () => {
  if (!diagDialog.open) diagDialog.showModal();
});
document.getElementById("diagnostics-close").addEventListener("click", () => diagDialog.close());
diagDialog.addEventListener("close", () => {
  if (diagnosisActive) stopDiagnostics("Stopped because diagnostics was closed. Partial results are available.");
});
diagRun.addEventListener("click", () => {
  if (!node || !running || booting) {
    diagStatus.textContent = "Start a firmware before measuring its performance.";
    return;
  }
  diagnosis = createAccuracySession({
    firmware: running.record.name,
    firmwareHash: running.record.hash,
    device: DEVICE, mode: accurate ? "accurate" : "performance", mhz,
    sampleRate: ac?.sampleRate || 44100, browser: navigator.userAgent || "unknown",
  });
  diagnosisSeq = running.seq;
  diagnosisActive = true;
  diagRun.disabled = true;
  diagExport.disabled = true;
  diagResult.textContent = "Collecting callback, LCD and audio-level statistics…";
  diagStatus.textContent = `Measuring ${DIAGNOSTIC_SECONDS}s of emulator audio…`;
  node.port.postMessage({ type: "monitor", enabled: true });
});
diagExport.addEventListener("click", () => {
  if (!diagnosis?.samples.length) return;
  const json = JSON.stringify(accuracyReport(diagnosis), null, 2) + "\n";
  const url = URL.createObjectURL(new Blob([json], { type: "application/json" }));
  const link = document.createElement("a");
  link.href = url;
  link.download = `fm1-accuracy-${diagnosis.mode}-${diagnosis.mhz}mhz.json`;
  link.click();
  setTimeout(() => URL.revokeObjectURL(url), 1000);
  announce("Diagnostic report exported; no audio or MIDI data included");
});

const saveStore = new SaveStore();
const memoryRecords = new Map();
const saveMessage = (text, failed = false) => {
  saveStatus.textContent = text;
  saveStatus.dataset.error = String(failed);
  if (failed) announce(text, true);
};
const saveFailure = error => saveMessage(`Not saved in browser: ${error.message}. Export a backup to keep your changes.`, true);

// Prefer the locally compiled WASM file (GitHub Actions includes it).
// The upload-only release can fetch the original author's public build until a local file is added.
async function loadEngine() {
  const sources = ["./fm1_web.wasm", "https://joakimeriksson.github.io/fm1-emu/fm1_web.wasm"];
  const errors = [];
  for (const url of sources) {
    try {
      const response = await fetch(url);
      if (!response.ok) throw new Error(`HTTP ${response.status}`);
      const bytes = await response.arrayBuffer();
      if (new DataView(bytes).getUint32(0, true) !== 0x6d736100) throw new Error("invalid WebAssembly binary");
      return bytes;
    } catch (error) { errors.push(`${url}: ${error.message}`); }
  }
  status.textContent = "Emulator engine unavailable. Check connection or deploy the built WASM.";
  document.getElementById("engine-error").hidden = false;
  throw new Error(errors.join("; "));
}
const wasm = await loadEngine();  // the worklet gets its own copy
// (the panel's instance runs no machine: its JIT import is never called)
const ui = (await WebAssembly.instantiate(wasm, { env: { fm1_jit_compile: () => 0 } })).instance.exports;
// the panel drawn: 0 the FM-1's, 1 the SMK-37's (setBoard; the canvas takes its size)
let W = 0, H = 0, board = -1;
const fitPanel = () => document.documentElement.style.setProperty("--above",
  `${Math.round(canvas.parentElement.getBoundingClientRect().top + scrollY)}px`);
{
  const ro = new ResizeObserver(fitPanel);
  ro.observe(document.querySelector("header"));
  ro.observe(document.querySelector(".bar"));
}
fitPanel();

let node = null;
let frame = null;            // the latest LCD + LEDs from the audio thread
const down = new Uint8Array(64);           // keys and buttons held (FM-1: matrix ids 0..40; SMK-37: keys 0..36, buttons 37..54)
const encClicks = new Int32Array(8);
const turning = new Uint32Array(9);
let scheduleDraw = () => {};
let pot = 900;
// SMK-37: its analog inputs as set (pads 0..15, faders 16..19, the pitch / mod wheel 20 / 21)
const PITCH_REST = 535, MOD_REST = 629;
const analog = new Uint16Array(23);
analog[20] = PITCH_REST;
analog[21] = MOD_REST;
function setBoard(b) {
  if (b === board) return;
  board = b;
  ui.panel_board(b);
  W = ui.panel_width();
  H = ui.panel_height();
  canvas.width = W;
  canvas.height = H;
  // the synth's width follows the room left below the header and the bar (index.html's canvas rule)
  document.documentElement.style.setProperty("--aspect", String(W / H));
  document.documentElement.dataset.board = b ? "smk37" : "fm1";
  down.fill(0);
  encClicks.fill(0);
  scheduleDraw();
}
setBoard(DEV.board);

// ---------------------------------------------------------------- firmware and clock
for (const f of catalog.firmwares) fwSel.add(new Option(`${f.name} ${f.version}`, f.id));
for (const c of new Set([...CLOCKS, mhz].sort((a, b) => a - b))) mhzSel.add(new Option(`${c} MHz`, c));
fwSel.value = fw.id;
mhzSel.value = mhz;
audioAheadSel.value = String(audioAhead);
audioAheadSel.addEventListener("change", () => {
  const chosen = Number(audioAheadSel.value);
  if (![256, 512, 1024].includes(chosen)) return;
  audioAhead = chosen;
  store("fm1.ahead", String(audioAhead));
  audioAheadSel.blur();
  announce(audioAhead === 1024 ? "Stable response: extra buffer reserve" :
    `Faster response: ${audioAhead} source frames of queue; may cause dropouts on slower devices`);
  if (node) return boot();
});
if (accurate && !ui.fm1_strict) accurate = false; // no silent inaccurate fallback with an older WASM.
modeSel.value = accurate ? "accurate" : "performance";
if (!ui.fm1_strict) {
  const option = [...modeSel.options].find(o => o.value === "accurate");
  if (option) option.disabled = true;
  modeSel.title = "Hardware Accuracy requires the new GitHub-built WebAssembly engine";
}

// Import-only edition: no catalog cards and no external firmware downloads.
const cards = new Map();
const MIDI_NOTES = {};

// Cached packages are pinned to their exact bytes. Checking for updates is explicit.
const packages = new Map();
let localFw = null;
class NeedFile extends Error {}
async function keptEntry(id) {
  try { return await saveStore.entry(DEV.key + id); }
  catch (error) { saveFailure(error); return null; }
}
async function packageOf(f) {
  if (f.id === "local") return { bytes: f.bytes, url: null };
  if (packages.has(f.id)) return packages.get(f.id);
  if (LOCKED_FIRMWARE) {
    if (f.id !== catalog.default) throw new Error("Only bundled RUPTURE firmware is allowed");
    const got = await fetchPackage(f);
    await verifyPinnedPackage(f, got.bytes, packageHash);
    packages.set(f.id, got);
    return got;
  }
  const kept = await keptEntry(f.id);
  if (kept) {
    const got = { bytes: kept.package, url: kept.url };
    packages.set(f.id, got);
    return got;
  }
  if (f.byo) {
    // Migration: old versions stored only uploaded stock packages, by catalog ID.
    let bytes;
    try { bytes = await saveStore.get("packages", f.id); } catch (error) { saveFailure(error); }
    if (!bytes) throw new NeedFile();
    const got = { bytes, url: null };
    packages.set(f.id, got);
    return got;
  }
  const got = await fetchPackage(f);
  packages.set(f.id, got);
  noteVersion(f, got.version);
  return got;
}
// A package that had gone came as the one the Hub lists now: show the version that runs.
function noteVersion(f, version) {
  if (!version || version === f.version) return;
  f.version = version;
  const opt = [...fwSel.options].find((o) => o.value === f.id);
  if (opt) opt.text = `${f.name} ${f.version}`;
  // The import-only edition has no firmware cards.
  if (f === fw) showFirmware();
}
const keptLocal = LOCKED_FIRMWARE ? null : await keptEntry("local");
if (keptLocal) {
  localFw = { id: "local", name: keptLocal.name, version: "", bytes: keptLocal.package };
  fwSel.add(new Option(`${localFw.name} (your file)`, "local"), 0);
  if (params.get("fw") === "local" || (!params.has("fw") && stored(DEV.key + "fw") === "local")) { fw = localFw; fwSel.value = "local"; }
}
for (const f of catalog.firmwares.filter(f => f.byo)) {
  try { await packageOf(f); } catch { /* user has not opened a stock package yet */ }
}
let bank = null;                           // a DX7 32-voice bank for the stock firmware, or null

let loadedUrl = null;                      // the package the running machine came from

function showFirmware() {
  const ready = (LOCKED_FIRMWARE && fw.id === catalog.default) || (fw.id === "local" && !!fw.bytes);
  title.textContent = ready ? fw.name : "No firmware loaded";
  about.textContent = LOCKED_FIRMWARE ? "Bundled and pinned RUPTURE firmware" : (ready ? fw.name : "Choose your own .fwsc firmware");
  document.getElementById("firmware-name").textContent = ready ? fw.name : "No firmware";
  document.getElementById("empty").hidden = ready;
  if (!node) startBtn.hidden = !ready;
  const q = new URLSearchParams(location.search);
  q.delete("fw");
  q.set("mhz", mhz);
  history.replaceState(null, "", `${location.pathname}?${q}`);
  document.getElementById("fullpage").href = "./";
}
showFirmware();
// RUPTURE edition preloads and checks its exact bundled FWSC on every visit.
// Sound starts only after a user gesture when required by browser autoplay policy.
if (LOCKED_FIRMWARE) {
  startBtn.disabled = true;
  status.textContent = "Checking bundled RUPTURE.fwsc…";
  packageOf(fw).then(() => {
    if (node) return;
    startBtn.disabled = false;
    status.textContent = "RUPTURE ready · press Start";
  }).catch(error => {
    startBtn.hidden = true;
    status.textContent = `RUPTURE could not load: ${error.message}`;
    announce(`RUPTURE could not load: ${error.message}`, true);
  });
}

fwSel.addEventListener("change", () => {
  if (LOCKED_FIRMWARE) { fwSel.value = fw.id; return; }
  fw = byId.get(fwSel.value) || localFw;
  endTryOut();
  loadedUrl = null;
  if (!EMBED) store(DEV.key + "fw", fw.id);
  showFirmware();
  fwSel.blur();                 // back to playing from the keyboard
  if (node) boot();
});
modeSel.addEventListener("change", () => {
  if (modeSel.value === "accurate" && !ui.fm1_strict) {
    modeSel.value = "performance";
    announce("Build the included WebAssembly engine to enable Hardware Accuracy", true);
    return;
  }
  const wasAccurate = accurate;
  accurate = modeSel.value === "accurate";
  store("fm1.mode", modeSel.value);
  if (accurate && !wasAccurate) {
    performanceMhz = mhz;
    store("fm1.performance.mhz", performanceMhz);
    mhz = 240;
    announce("Hardware Accuracy: measured CPU costs, LCD timing and XIP cache; slower than Performance");
  } else if (!accurate && wasAccurate) {
    mhz = performanceMhz;
    announce("Performance mode: JIT enabled for smoother playback");
  }
  mhzSel.value = String(mhz);
  store("fm1.mhz", mhz);
  showFirmware();
  if (node) boot();
});
mhzSel.addEventListener("change", () => {
  mhz = Number(mhzSel.value);
  store("fm1.mhz", mhz);
  if (!accurate) {
    performanceMhz = mhz;
    store("fm1.performance.mhz", mhz);
  }
  showFirmware();
  mhzSel.blur();
  if (node) boot();
});

// a package from this computer: the file picker or a drop on the panel. With M-VAVE's entry
// chosen, or a file named like theirs, it is that entry's copy and is kept for next time.
async function openLocal(file) {
  assertFirmwareImportAllowed(LOCKED_FIRMWARE);
  if (!file) return;
  if (!/\.fwsc$/i.test(file.name)) { status.textContent = "Choose a .fwsc file"; return; }
  if (file.size < 960 || file.size > 8 * 1024 * 1024) {
    status.textContent = "Firmware file size is invalid";
    return;
  }
  localFw = { id: "local", name: file.name.replace(/\.fwsc$/i, ""), version: "", bytes: new Uint8Array(await file.arrayBuffer()) };
  let opt = [...fwSel.options].find((o) => o.value === "local");
  if (!opt) fwSel.add(opt = new Option("", "local"), 0);
  opt.text = `${localFw.name} (your file)`;
  fwSel.value = "local";
  fw = localFw;
  if (!EMBED) store(DEV.key + "fw", "local");
  loadedUrl = null;
  showFirmware();
  if (node) boot();
  else status.textContent = `${file.name} ready: click Start`;
}
openInput?.addEventListener("change", () => {
  openLocal(openInput.files[0]).catch(error => { status.textContent = `Import error: ${error.message}`; });
  openInput.value = "";
});
document.getElementById("import-stage")?.addEventListener("click", () => {
  if (!LOCKED_FIRMWARE) openInput?.click();
});
bankInput.addEventListener("change", async () => {
  const file = bankInput.files[0];
  bankInput.value = "";
  if (!file) return;
  bank = new Uint8Array(await file.arrayBuffer());
  bankName.textContent = file.name;
  if (node) boot();
});
const stage = document.querySelector(".stage");
stage.addEventListener("dragover", (e) => {
  e.preventDefault();
  if (LOCKED_FIRMWARE) e.dataTransfer.dropEffect = "none";
});
stage.addEventListener("drop", (e) => {
  e.preventDefault();
  if (LOCKED_FIRMWARE) { announce("This edition always runs the bundled RUPTURE firmware", true); return; }
  const file = [...e.dataTransfer.files].find((f) => /\.(fwsc|syx)$/i.test(f.name));
  if (!file) return;
  if (/\.syx$/i.test(file.name)) {
    file.arrayBuffer().then((b) => { bank = new Uint8Array(b); bankName.textContent = file.name; if (node) boot(); });
  } else {
    openLocal(file).catch(error => { status.textContent = `Import error: ${error.message}`; });
  }
});

// ---------------------------------------------------------------- event-driven panel rendering
// The emulated LCD/LED state normally arrives at ~30 Hz. The hardware shell is
// static; redrawing it at 60+ Hz was an avoidable 4 MB/frame memory copy.
let imageData = null;
let imageBuffer = null;
let imagePointer = -1;
function draw() {
  const memory = ui.memory.buffer;
  if (frame) {
    if (frame.lcd) new Uint16Array(memory, ui.panel_lcd(), 240 * 240).set(frame.lcd);
    if (frame.leds) new Float32Array(memory, ui.panel_leds(), 41).set(frame.leds);
    if (frame.pads) for (let p = 0; p < 16; p++) ui.panel_set(5, p, frame.pads[p]);
    frame = null;
  }
  for (let i = 0; i < 64; i++) ui.panel_set(0, i, down[i]);
  for (let i = 0; i < 8; i++) ui.panel_set(1, i, encClicks[i]);
  ui.panel_set(2, 0, pot);
  let animating = false;
  for (let i = 0; i < 9; i++) {
    ui.panel_set(3, i, turning[i]);
    if (turning[i]) { turning[i]--; animating = true; }
  }
  if (board === 1) for (let i = 0; i < 23; i++) ui.panel_set(4, i, analog[i]);
  const pointer = ui.panel_draw();
  const outputBuffer = ui.memory.buffer; // panel_draw may have grown WASM memory
  if (!imageData || imageBuffer !== outputBuffer || imagePointer !== pointer ||
      imageData.width !== W || imageData.height !== H) {
    imageData = new ImageData(new Uint8ClampedArray(outputBuffer, pointer, W * H * 4), W, H);
    imageBuffer = outputBuffer;
    imagePointer = pointer;
  }
  ctx.putImageData(imageData, 0, 0);
  return animating;
}
const panelScheduler = createPanelScheduler({ requestFrame: requestAnimationFrame, render: draw });
scheduleDraw = () => panelScheduler.invalidate();
scheduleDraw();

// ---------------------------------------------------------------- input to the machine
const send = (m) => node && node.port.postMessage(m);
function setKey(id, on) {
  if (down[id] === (on ? 1 : 0)) return;
  down[id] = on ? 1 : 0;
  scheduleDraw();
  send({ type: "key", id, down: on });
}
function turn(i, clicks) {
  encClicks[i] += clicks;
  turning[i] = 10;
  scheduleDraw();
  send({ type: "enc", enc: ui.panel_enc_id(i), clicks });
}
function setPot(v) {
  pot = Math.max(0, Math.min(1023, Math.round(v)));
  turning[7] = 10;                           // (the FM-1's knob 7: its pot; the SMK-37's K8)
  scheduleDraw();
  send({ type: "pot", value: pot });
}
// SMK-37: an analog input (a pad's force, a fader, a wheel)
function setAnalog(i, v) {
  v = Math.max(0, Math.min(1023, Math.round(v)));
  if (analog[i] === v) return;
  analog[i] = v;
  scheduleDraw();
  send({ type: "analog", idx: i, value: v });
}
// SMK-37: the velocity of the next keys, as the time between a key's two contacts: 0 (softest)
// .. 1 (hardest) -> 80 ms .. 2 ms (the stock curve: full below ~4 ms, the least above ~100 ms)
function setVelocity(f) {
  send({ type: "vel", us: Math.round(Math.exp(Math.log(80000) + (Math.log(2000) - Math.log(80000)) * Math.max(0, Math.min(1, f)))) });
}
const NKEY = 37, BTN0 = 37;

// pointer: keys and buttons held while pressed (drag across keys: glissando), knobs dragged
// up / down or scrolled, MASTER as a pot. Every pointer on its own, so on a touch screen
// several fingers play a chord, or hold a key while one turns a knob.
const at = (e) => {
  const r = canvas.getBoundingClientRect();
  return [(e.clientX - r.left) * (W / r.width), (e.clientY - r.top) * (H / r.height)];
};
const holds = new Uint8Array(64);          // pointers holding each key / button
function press(id) { if (holds[id]++ === 0) setKey(id, true); }
function unpress(id) { if (holds[id] && --holds[id] === 0) setKey(id, false); }
const pointers = new Map();                // pointerId -> { grab, lastY, acc, held }
canvas.addEventListener("pointerdown", (e) => {
  const [x, y] = at(e);
  const p = { grab: ui.panel_hit(x, y), lastY: y, acc: 0, held: -1 };
  pointers.set(e.pointerId, p);
  canvas.setPointerCapture(e.pointerId);
  if (p.grab >= 100 && p.grab < 200) {
    p.held = p.grab - 100;
    // SMK-37: a key hit further down sounds harder
    if (board === 1 && p.held < NKEY) setVelocity(ui.panel_hit_y());
    press(p.held);
  } else if (board === 1 && p.grab >= 200) slide(p, y);
});
// SMK-37: a pad pressed (harder the lower it is hit), a fader or a wheel dragged
function slide(p, y) {
  if (p.grab < 300) setAnalog(p.grab - 200, 700);
  else if (p.grab < 400) setAnalog(16 + p.grab - 300, ui.panel_slide(p.grab - 300, y));
  else setAnalog(p.grab === 400 ? 20 : 21, ui.panel_slide(p.grab === 400 ? 4 : 5, y));
}
canvas.addEventListener("pointermove", (e) => {
  const p = pointers.get(e.pointerId);
  if (!p || p.grab < 0) return;
  const [x, y] = at(e);
  const keys = board === 1 ? [0, NKEY] : [14, 41];
  if (p.grab >= 100 && p.grab < 200 && p.held >= keys[0] && p.held < keys[1]) {
    const h = ui.panel_hit(x, y);              // glissando over the note keys
    if (h >= 100 + keys[0] && h < 100 + keys[1] && h - 100 !== p.held) {
      unpress(p.held);
      p.held = h - 100;
      if (board === 1) setVelocity(ui.panel_hit_y());
      press(p.held);
    }
  } else if (board === 1 && p.grab >= 300) {
    slide(p, y);
  } else if (p.grab >= 0 && p.grab < (board === 1 ? 8 : 7)) {
    p.acc += p.lastY - y;
    while (p.acc >= 12) { p.acc -= 12; turn(p.grab, 1); }
    while (p.acc <= -12) { p.acc += 12; turn(p.grab, -1); }
  } else if (board === 0 && p.grab === 7) {
    setPot(pot + (p.lastY - y) * 4);
  }
  p.lastY = y;
});
const release = (e) => {
  const p = pointers.get(e.pointerId);
  if (!p) return;
  if (p.held >= 0) unpress(p.held);
  if (board === 1 && p.grab >= 200 && p.grab < 216) setAnalog(p.grab - 200, 0);   // a pad let go
  if (board === 1 && p.grab === 400) setAnalog(20, PITCH_REST);                   // the pitch wheel springs back
  pointers.delete(e.pointerId);
};
canvas.addEventListener("pointerup", release);
canvas.addEventListener("pointercancel", release);
canvas.addEventListener("lostpointercapture", release);
let wheelAcc = 0;
canvas.addEventListener("wheel", (e) => {
  const [x, y] = at(e);
  const h = ui.panel_hit(x, y);
  if (board === 1 && h >= 300 && h < 304) {          // SMK-37: a fader scrolled
    e.preventDefault();
    setAnalog(16 + h - 300, analog[16 + h - 300] - e.deltaY * 2);
    return;
  }
  if (h < 0 || h > 7) return;
  e.preventDefault();
  if (board === 0 && h === 7) { setPot(pot - e.deltaY * 0.5); return; }
  wheelAcc -= e.deltaY;
  while (wheelAcc >= 40) { wheelAcc -= 40; turn(h, 1); }
  while (wheelAcc <= -40) { wheelAcc += 40; turn(h, -1); }
}, { passive: false });

// Rebindable keyboard shortcuts (separate from emulator, no change to firmware handling).
import { createShortcuts } from "./shortcuts.js";
const heldPhysical = new Map();
function shortcutDown(action) {
  if (action.startsWith("note-")) { const id = 21 + Number(action.slice(5)); setKey(id, true); return; }
  if (action.startsWith("button-")) { setKey(Number(action.slice(7)), true); return; }
  if (action.startsWith("enc-")) {
    const [, index, sign] = action.split("-");
    turn(Number(index), sign === "plus" ? 1 : -1);
    return;
  }
  if (action === "master-plus") setPot(pot + 20);
  if (action === "master-minus") setPot(pot - 20);
}
function shortcutUp(action) {
  if (action.startsWith("note-")) setKey(21 + Number(action.slice(5)), false);
  if (action.startsWith("button-")) setKey(Number(action.slice(7)), false);
}
const shortcuts = createShortcuts({
  dialog: document.getElementById("shortcuts-dialog"),
  onDown: shortcutDown,
  onUp: shortcutUp,
  shouldIgnore: (target) => !!target?.closest?.("button,input,select,textarea,[contenteditable='true'],dialog"),
});
addEventListener("keydown", event => {
  if (event.ctrlKey || event.metaKey || event.altKey || shortcuts.isCapturing()) return;
  if (shortcuts.shouldIgnore(event.target)) return;
  const action = shortcuts.actionFor(event.code);
  if (!action) return;
  event.preventDefault();
  if (action.startsWith("enc-") || action.startsWith("master-")) {
    shortcutDown(action); // autorepeat is desirable for encoders / MASTER
  } else if (!event.repeat) {
    heldPhysical.set(event.code, action);
    shortcutDown(action);
  }
});
addEventListener("keyup", event => {
  const action = heldPhysical.get(event.code);
  if (!action) return;
  heldPhysical.delete(event.code);
  shortcutUp(action);
});
addEventListener("blur", () => {
  for (const action of heldPhysical.values()) shortcutUp(action);
  heldPhysical.clear();
});
document.getElementById("keys-open").addEventListener("click", () => shortcuts.open());
document.getElementById("keys-close").addEventListener("click", () => shortcuts.close());
document.getElementById("keys-done").addEventListener("click", () => shortcuts.close());
document.getElementById("keys-reset").addEventListener("click", () => shortcuts.reset());

// ---------------------------------------------------------------- MIDI in (Web MIDI)
// The chosen input(s) play the synth through its MIDI IN jack (UART1 at 31250 baud in the
// emulator). Access is asked for only once an input is picked (Chrome asks the visitor), and on
// Start when one was picked on an earlier visit. Channel: channel messages rewritten to one
// channel (0 = as sent).
const midiSel = document.getElementById("midi");
const midiChSel = document.getElementById("midich");
const midiAct = document.getElementById("midiact");
let midi = null, midiAsked = null, midiActOff = 0;
let midiCh = Number(stored("fm1.midich") || 0);
for (let c = 1; c <= 16; c++) midiChSel.add(new Option(String(c), c));
midiChSel.value = midiCh;
const midiWant = () => stored("fm1.midi") || "";
if (!navigator.requestMIDIAccess) {
  midiSel.disabled = midiChSel.disabled = true;
  document.getElementById("midiin").title = "this browser has no Web MIDI (Chrome, Edge and Firefox do)";
}
function midiMessage(e) {
  const d = e.data;
  if (!d.length || !node) return;
  const bytes = Array.from(d);
  if (midiCh && bytes[0] >= 0x80 && bytes[0] < 0xf0) bytes[0] = (bytes[0] & 0xf0) | (midiCh - 1);
  if (board === 1) {
    // the SMK-37 has no MIDI in jack: notes C3..C6 play its keys (the velocity as the time between
    // the contacts), the pitch bend and the mod wheel its wheels
    const st = bytes[0] & 0xf0, k = bytes[1] - 48;
    if ((st === 0x90 || st === 0x80) && k >= 0 && k < NKEY) {
      const on = st === 0x90 && bytes[2] > 0;
      if (on) setVelocity((bytes[2] - 1) / 126);
      setKey(k, on);
    } else if (st === 0xe0) {
      const bend = ((bytes[2] << 7) | bytes[1]) - 8192;
      setAnalog(20, PITCH_REST + (bend >= 0 ? bend / 8191 * (640 - PITCH_REST) : bend / 8192 * (PITCH_REST - 445)));
    } else if (st === 0xb0 && bytes[1] === 1) {
      setAnalog(21, MOD_REST - bytes[2] / 127 * (MOD_REST - 474));
    }
  } else {
    send({ type: "midi", bytes });
  }
  if (bytes[0] < 0xf8) {
    midiAct.classList.add("on");
    clearTimeout(midiActOff);
    midiActOff = setTimeout(() => midiAct.classList.remove("on"), 120);
  }
}
// listen to the wanted inputs only; the list follows devices coming and going
function midiConnect() {
  if (!midi) return;
  const want = midiSel.value;
  const names = [...midi.inputs.values()].map((i) => i.name);
  for (const o of [...midiSel.options]) if (o.value !== "" && o.value !== "*" && !names.includes(o.value)) o.remove();
  for (const n of names) if (![...midiSel.options].some((o) => o.value === n)) midiSel.add(new Option(n, n));
  if ([...midiSel.options].some((o) => o.value === want)) midiSel.value = want;
  for (const input of midi.inputs.values())
    input.onmidimessage = want === "*" || want === input.name ? midiMessage : null;
}
async function midiOpen() {
  if (midi || !navigator.requestMIDIAccess) return midiConnect();
  midiAsked ||= navigator.requestMIDIAccess().then((m) => {
    midi = m;
    midi.onstatechange = midiConnect;
  }, () => {
    midiAsked = null;
    midiSel.value = "";
    status.textContent = "MIDI: access was not given";
  });
  await midiAsked;
  midiConnect();
}
{
  const want = midiWant();
  if (want && want !== "*") midiSel.add(new Option(want, want));
  midiSel.value = want;
}
midiSel.addEventListener("change", () => {
  store("fm1.midi", midiSel.value);
  if (midiSel.value) midiOpen(); else midiConnect();
});
midiChSel.addEventListener("change", () => {
  midiCh = Number(midiChSel.value);
  store("fm1.midich", midiCh);
});

// ---------------------------------------------------------------- is the device keeping up?
// Two measures, per second: the load (the share of the audio thread's time the emulation takes,
// timed in the worklet; over ~100% and the sound must break up) and the drift of the audio
// clock against the wall clock (getOutputTimestamp: the output falls behind when quanta come
// late, also with a load below 100% when the buffer is too small for this device's hiccups).
// Some seconds of either, once the JIT has warmed up, bring up a warning that offers the fix.
// The speed shown is the emulator's own: emulated time per time spent emulating (as
// `node web/bench.mjs` and `fm1-live --bench` print it); at 1x a device just keeps up.
const warnBox = document.getElementById("perfwarn");
const perf = {
  reset() {
    this.busy = 0; this.span = 0; this.emulated = 0; this.idle = 0; this.idleShare = 0; this.worst = 0;
    this.secs = 0; this.bad = 0; this.drops = 0; this.loads = []; this.speeds = []; this.drift = 0;
    this.stamp = null; this.jit = null; this.dismissed = false;
    warnBox.hidden = true;
  },
  add(m) {
    this.busy += m.busy;
    this.span += (m.quanta * m.frames) / 44.1;     // ms of audio those quanta stand for
    this.emulated += m.emulated ?? 0;              // ms of emulated time they ran
    this.idle += m.idle ?? 0;                      // of which idle skipping jumped over
    if (m.worst > this.worst) this.worst = m.worst;
    this.jit = m.jit;
  },
  load() { return this.loads.length ? this.loads[this.loads.length - 1] : 0; },
  loadText() { return this.loads.length ? `${Math.round(100 * this.load())}%` : "…"; },
  speed() { return this.speeds.length ? this.speeds[this.speeds.length - 1] : NaN; },
  speedText() { return Number.isFinite(this.speed()) ? `${this.speed().toFixed(this.speed() < 10 ? 1 : 0)}x real time` : "…"; },
  // once a second: close the window, judge it
  second(m) {
    if (this.span > 0) this.loads.push(this.busy / this.span);
    // (a whole-millisecond worklet clock: a second's quanta still add up to a fair figure)
    if (this.busy > 0 && this.emulated > 0) this.speeds.push(this.emulated / this.busy);
    if (this.emulated > 0) this.idleShare = Math.min(1, this.idle / this.emulated);
    if (this.loads.length > 30) this.loads.shift();
    if (this.speeds.length > 30) this.speeds.shift();
    this.busy = 0; this.span = 0; this.emulated = 0; this.idle = 0;
    const ts = ac.getOutputTimestamp?.();
    let drift = 0;
    if (ts && ts.contextTime > 0) {
      if (this.stamp) {
        const wall = (ts.performanceTime - this.stamp.performanceTime) / 1000;
        if (wall > 0.5) drift = 1 - (ts.contextTime - this.stamp.contextTime) / wall;
      }
      this.stamp = ts;
    }
    this.drift = drift;
    if (++this.secs <= 3 || m.halted) return;      // (the first seconds: booting, compiling)
    const over = this.load() > 0.95, late = drift > 0.02;
    if (late) this.drops++;
    this.bad = over || late ? this.bad + 1 : 0;
    if (this.bad >= 3 && warnBox.hidden && !this.dismissed) this.warn(over);
  },
  // A lower clock that brings the load down to ~80%, if one does. The cost of emulating a
  // second is a fixed part (about a quarter at the clocks offered: the peripherals, the audio, the
  // JIT's dispatch) and the cycles the firmware runs, which a lower clock cuts only where the
  // firmware runs more of them than the lower clock has: the time it idles is skipped (fm1_soc's
  // skip_idle), so a firmware that mostly idles (M-VAVE's: 70-80% of the time at 120-240 MHz)
  // does the same work at any clock, and a lower one does not help it (measured: 2.6x real time
  // at 60 MHz, 2.4x at 240; Felucca 5.0x at 60 MHz, 2.0x at 240).
  lowerClock(load) {
    const FIXED = 0.25, busy = mhz * (1 - this.idleShare);   // MHz of cycles the firmware runs
    const at = (c) => load * (FIXED + (1 - FIXED) * Math.min(busy, c) / Math.max(busy, 1));
    const lower = CLOCKS.filter((c) => c < mhz && at(c) <= 0.8);
    const pick = lower.length ? lower[lower.length - 1] : CLOCKS.find((c) => c < mhz);
    return pick && at(pick) < 0.85 * load ? pick : null;     // only where it clearly helps
  },
  warn(over) {
    const load = this.load();
    const pick = over && !accurate ? this.lowerClock(load) : null;
    const text = document.getElementById("perfwarn-text");
    const fix = document.getElementById("perfwarn-fix");
    // (a debugger attached, as DevTools open, runs the WebAssembly 3-4x slower)
    const devtools = " If the browser's developer tools are open, close them: they slow the emulator down 3-4x.";
    fix.hidden = false;
    if (accurate && over) {
      text.textContent = `Hardware Accuracy uses measured CPU instruction costs and an interpreter. This device needs ${Math.round(100 * load)}% of the audio thread at ${mhz} MHz; switch to Performance for live audio.`;
      fix.textContent = "Use Performance";
      fix.onclick = () => {
        modeSel.value = "performance";
        modeSel.dispatchEvent(new Event("change"));
      };
    } else if (over && pick) {
      text.textContent = `This device can't run ${fw.name} at ${mhz} MHz in real time (it needs ${Math.round(100 * load)}% ` +
        `of the audio thread), so the sound breaks up. It needs less at a lower clock.` + devtools;
      fix.textContent = `Use ${pick} MHz`;
      fix.onclick = () => { setClock(pick); };
    } else if (over) {
      // (a lower clock exists but would not help: say why)
      const why = !CLOCKS.some((c) => c < mhz) ? "" : this.idleShare >= 0.3
        ? " A lower clock would not help: the emulator skips the time this firmware idles, so it costs about the same at any clock."
        : " A lower clock would help only a little.";
      text.textContent = `This device can't run ${fw.name} at ${mhz} MHz in real time (it needs ` +
        `${Math.round(100 * load)}% of the audio thread), so the sound breaks up.` + why + devtools +
        ` A faster device or the desktop version will play it.`;
      fix.hidden = true;
    } else if (latency !== "playback") {
      text.textContent = `The sound drops out although the emulator has time to spare (load ${Math.round(100 * load)}%): ` +
        `this device's audio buffer is too small for it. A larger one adds a little delay to the keys.`;
      fix.textContent = "Use a larger audio buffer";
      fix.onclick = () => { setLatency("playback"); };
    } else {
      text.textContent = `The sound drops out on this device, even with the larger audio buffer (load ${Math.round(100 * load)}%).`;
      fix.hidden = true;
    }
    warnBox.hidden = false;
  },
  // for a bug report: what the device is and how it kept up
  report() {
    const r = (v, d = 0) => (Number.isFinite(v) ? v.toFixed(d) : "?");
    const loads = this.loads.map((l) => Math.round(100 * l)), jit = this.jit;
    return [
      `FM-1 emulator report (${location.origin}${location.pathname})`,
      `firmware: ${fw.id} ${fw.version || ""} · clock ${mhz} MHz · JIT ${params.get("jit") === "0" ? "off" : "on"}`,
      `browser: ${navigator.userAgent}`,
      `cores: ${navigator.hardwareConcurrency ?? "?"} · memory: ${navigator.deviceMemory ?? "?"} GB`,
      `audio: ${ac ? `FM-1 44100 Hz → output ${ac.sampleRate} Hz${ac.sampleRate === 44100 ? ' (native)' : ' (continuous conversion)'}, buffer "${latency}", base latency ${r(1000 * (ac.baseLatency ?? NaN), 1)} ms, ` +
        `output latency ${r(1000 * (ac.outputLatency ?? NaN), 1)} ms` : "not started"}`,
      `speed (emulated time per time spent emulating, last ${this.speeds.length} s): ${this.speeds.map((v) => r(v, 2)).join(" ")}x real time`,
      `load (last ${loads.length} s): ${loads.join(" ")}% · worst quantum since the audio started ${r(this.worst, 1)} ms of ${r(128 / 44.1, 1)}`,
      `audio clock drift: ${r(100 * this.drift, 1)}% · seconds with drops: ${this.drops} · idle skipped: ${r(100 * this.idleShare)}% of the time`,
      `jit: ${jit ? `${jit.regions} regions compiled, ${jit.failed} failed${jit.error ? ` (${jit.error})` : ""} · ` +
        `compiling took ${r(jit.compileMs)} ms, the longest ${r(jit.compileMaxMs, 1)} ms` : "?"}`,
      `emulated: ${frame ? r(frame.ms / 1000, 1) : "?"} s · short audio quanta: ${frame?.short ?? "?"}`,
    ].join("\n");
  },
};
document.getElementById("perfwarn-copy").addEventListener("click", async (e) => {
  const b = e.currentTarget;
  try {
    await navigator.clipboard.writeText(perf.report());
    b.textContent = "Copied";
  } catch {
    prompt("Copy this report:", perf.report());     // (no clipboard access: let them copy it)
  }
  setTimeout(() => { b.textContent = "Copy a report"; }, 2000);
});
document.getElementById("perfwarn-close").addEventListener("click", () => {
  warnBox.hidden = true;
  perf.dismissed = true;
});
function setClock(c) {
  mhz = c;
  if (![...mhzSel.options].some((o) => Number(o.value) === c)) mhzSel.add(new Option(`${c} MHz`, c));
  mhzSel.value = c;
  store("fm1.mhz", c);
  showFirmware();
  boot();
}
async function setLatency(l) {
  await bootQueue;
  await flushRunning(true);
  latency = l;
  store("fm1.latency", l);
  // a larger buffer needs a new audio context: rebuild the graph, boot again
  node.port.onmessage = null;
  node.disconnect();
  running = null;
  await ac.close();
  await startAudio();
  await boot();
  await ac.resume();
}

// ---------------------------------------------------------------- audio: start on a click
let ac = null, last = null, bootSeq = 0, booting = 0;

let bootQueue = Promise.resolve(), running = null, requestId = 0;
const sessions = new Map(), bootWaiters = new Map(), snapshotWaiters = new Map();

function sameSectors(a, b) {
  return a.length === b.length && a.every((s, i) => {
    if (s.at !== b[i].at) return false;
    const left = new DataView(s.data.buffer, s.data.byteOffset, s.data.byteLength);
    const right = new DataView(b[i].data.buffer, b[i].data.byteOffset, b[i].data.byteLength);
    for (let at = 0; at < s.data.length; at += 4) if (left.getUint32(at) !== right.getUint32(at)) return false;
    return true;
  });
}
function saveSnapshot(session, flash, flashRevision) {
  session.flash = flash;
  const sectors = changedSectors(session.base, flash);
  if (session.ephemeral) {                 // a try-out: kept for an export, never stored
    session.record = { ...session.record, sectors };
    if (running === session) saveMessage("A try-out: not saved in this browser (Export a backup keeps it)");
    return Promise.resolve();
  }
  session.record = { ...session.record, sectors };
  memoryRecords.set(session.record.hash, session.record);
  if (!session.forceSave && session.revision && sameSectors(session.persistedSectors || [], sectors)) {
    session.pending = Promise.resolve(session.pending).then(() => saveStore.remember(session.record.hash, session.entry));
    session.pending.then(() => { if (running === session) saveMessage("Saved in this browser"); })
      .catch(error => { if (running === session) saveFailure(error); });
    return session.pending;
  }
  session.record.unsaved = true;
  if (running === session) saveMessage("Saving…");
  const record = session.record;
  session.pending = (session.pending || Promise.resolve()).catch(() => {}).then(async () => {
    const { unsaved: _unsaved, ...stored } = record;
    const revision = await saveStore.save({ ...stored, revision: session.revision }, session.entry);
    session.revision = revision;
    record.unsaved = false;
    session.persistedSectors = record.sectors;
    session.forceSave = false;
    session.record.revision = revision;
    // Keep the newest memory snapshot, even when an earlier transaction has just completed.
    const latest = memoryRecords.get(record.hash);
    if (latest) latest.revision = revision;
    if (running === session && session.record === record && session.observedRevision === flashRevision) saveMessage("Saved in this browser");
  });
  session.pending.catch(error => { if (running === session) saveFailure(error); });
  return session.pending;
}

async function flushRunning(pause = false) {
  const session = running;
  if (!session) return;
  if (ac.state === "suspended") await ac.resume();
  const request = ++requestId;
  const response = new Promise((resolve, reject) => {
    const timer = setTimeout(() => {
      snapshotWaiters.delete(request);
      node?.port.postMessage({ type: "cancel-snapshot", request });
      reject(new Error("flash snapshot timed out"));
    }, 10000);
    snapshotWaiters.set(request, { seq: session.seq, resolve, reject, timer });
  });
  node.port.postMessage({ type: "snapshot", seq: session.seq, request, pause });
  await response;
  // A storage failure leaves the snapshot in memory, available for export and retry.
  await session.pending?.catch(() => {});
}

// Serialize machine replacement, including the snapshot of the old machine. Requests that
// became obsolete while downloading are skipped; every actual boot receives an acknowledgement.
function boot(options = {}) {
  if (diagnosisActive) stopDiagnostics("Stopped: firmware or clock was changed. Partial report available.");
  const seq = ++bootSeq, f = fw, clock = mhz;
  const task = async () => {
    if (seq !== bootSeq) return false;
    const importedBank = bank;
    let got, trying = null;
    try {
      got = await packageOf(f);
      if (tryOut && tryOut.fw === f && !options.imported) {
        tell("loading");
        try {
          trying = tryOut.files ||= await loadTryOut(tryOut, fetch);
        } catch (error) {             // (the firmware boots as it would without the try-out)
          tell("error", error.message);
          tryOutError = `try-out: ${error.message}`;
          endTryOut();
        }
      }
      const hash = await packageHash(got.bytes);
      let record = trying ? null : memoryRecords.get(hash);
      if (!record && !trying) {
        try { record = await saveStore.load(hash); } catch (error) { saveFailure(error); }
      }
      if (seq !== bootSeq) return false;
      await flushRunning(true);
      // flushRunning may have updated this firmware's most recent snapshot.
      record = trying ? null : options.imported
        ? { ...options.imported, revision: memoryRecords.get(hash)?.revision || options.imported.revision }
        : memoryRecords.get(hash) || record;
      if (options.reset) record = { ...record, sectors: [] };
      const sectors = trying ? trying.sectors : record?.sectors || [];
      const session = { seq, ephemeral: !!trying, forceSave: !!(options.reset || options.imported || record?.unsaved),
        persistedSectors: record?.sectors || [], entry: DEV.key + f.id, revision: record?.revision || 0,
        record: { version: 1, hash, name: f.name, byo: !!(f.byo || record?.byo), device: DEVICE, package: got.bytes, url: got.url,
          sectors, revision: record?.revision || 0 } };
      sessions.set(seq, session);
      loadedUrl = got.url;
      showFirmware();
      down.fill(0); holds.fill(0); pointers.clear(); encClicks.fill(0);
      frame = last = null;
      scheduleDraw();
      booting = seq;
      perf.reset();
      const ready = new Promise((resolve, reject) => {
        const timer = setTimeout(() => {
          bootWaiters.delete(seq);
          running = null;
          node?.port.postMessage({ type: "cancel-boot", seq });
          reject(new Error("firmware boot timed out"));
        }, 30000);
        bootWaiters.set(seq, { resolve, reject, timer });
      });
      node.port.postMessage({ type: "boot", seq, wasm: wasm.slice(0), package: got.bytes.slice().buffer,
        sectors: packSectors(sectors).buffer, syx: importedBank ? importedBank.slice().buffer : null,
        script: trying?.script ? new TextEncoder().encode(trying.script).buffer : null,
        mhz: clock, accurate, jit: !accurate && params.get("jit") !== "0",
        // (?ahead=0: each quantum emulates only for its own frames, as before the even pace)
        ahead: audioAhead });
      status.textContent = `booting ${f.name}…`;
      if (trying) { tryOut.told = null; tell("booting"); }
      await ready;
      if (bank === importedBank) bank = null; // imports seed flash once; saved edits win on later boots
      return true;
    } catch (error) {
      if (bank === importedBank) bank = null;
      booting = 0;
      node?.port.postMessage({ type: "resume" });
      if (seq === bootSeq) status.textContent = error instanceof NeedFile
        ? `${f.name}: open your ${DEV.stock} to boot it` : `${f.name}: ${error.message}`;
      if (trying) tell("error", error.message);
      return false;
    }
  };
  bootQueue = bootQueue.then(task, task);
  return bootQueue;
}

// the audio graph: a context with the chosen buffer, the emulator's worklet node in it
async function startAudio(auto = false) {
  // Request the FM-1's native 44.1 kHz rate. Some mobile browsers reject or
  // override it; in that case the AudioWorklet continuously converts the FM-1
  // 44.1 kHz samples to the output context's actual sampleRate.
  try {
    ac = new AudioContext({ sampleRate: 44100, latencyHint: latency });
  } catch (error) {
    if (!(error instanceof Error) && !error?.name) throw error;
    ac = new AudioContext({ latencyHint: latency });
  }
  // (auto: without a click, only where the browser lets this page play: else the context is closed again)
  if (auto && ac.state !== "running" && !await new Promise((resolve) => {
    const timer = setTimeout(() => resolve(false), 500);
    ac.onstatechange = () => { if (ac.state === "running") { clearTimeout(timer); resolve(true); } };
  })) {
    ac.onstatechange = null;
    await ac.close();
    ac = null;
    return false;
  }
  await ac.audioWorklet.addModule("worklet.js");
  node = new AudioWorkletNode(ac, "fm1", { numberOfInputs: 0, outputChannelCount: [2] });
  node.connect(ac.destination);
  node.port.onmessage = onWorklet;
  perf.reset();
  // Boot now waits for a worklet acknowledgement; let its thread run even on browsers
  // that create an AudioContext suspended until the Start gesture is resumed explicitly.
  await ac.resume();
  return true;
}

// what the audio thread sends: the screen and LEDs with its timing, boot and error news
function onWorklet(e) {
  const m = e.data;
  if (m.type === "error") {
    if (m.fatal && running?.seq === m.seq) {
      running = null;
      for (const [id, pending] of snapshotWaiters) if (pending.seq === m.seq) {
        clearTimeout(pending.timer); snapshotWaiters.delete(id); pending.resolve();
      }
    }
    const waiter = bootWaiters.get(m.seq);
    if (waiter) { clearTimeout(waiter.timer); bootWaiters.delete(m.seq); waiter.reject(new Error(m.message)); }
    if (m.seq === bootSeq) status.textContent = `${fw.name}: ${m.message}`;
    return;
  }
  if (m.type === "flash-changed") {
    const session = sessions.get(m.seq);
    if (session) session.observedRevision = m.revision;
    if (session === running && !session.ephemeral) saveMessage("Saving…");
    return;
  }
  if (m.type === "snapshot") {
    const session = sessions.get(m.seq);
    if (session && m.flash) {
      session.observedRevision = m.revision;
      saveSnapshot(session, m.flash, m.revision);
    }
    const waiter = snapshotWaiters.get(m.request);
    if (waiter?.seq === m.seq) {
      clearTimeout(waiter.timer); snapshotWaiters.delete(m.request);
      if (m.error) waiter.reject(new Error(m.error)); else waiter.resolve();
    }
    return;
  }
  if (m.type === "booted") {
    const session = sessions.get(m.seq), waiter = bootWaiters.get(m.seq);
    if (!session || !waiter) return;
    running = session;
    for (const key of sessions.keys()) if (key !== m.seq) sessions.delete(key);
    session.base = m.base;
    session.observedRevision = m.revision;
    saveSnapshot(session, m.flash, m.revision);
    clearTimeout(waiter.timer); bootWaiters.delete(m.seq); waiter.resolve();
    if (m.seq === bootSeq) {
      booting = 0;
      setBoard(m.board);
      if (m.board === 1) for (let i = 16; i < 22; i++) send({ type: "analog", idx: i, value: analog[i] });
    }
    return;
  }
  if (m.type !== "frame" || booting || m.seq !== bootSeq) return;
  if (running?.ephemeral && tryOut?.files && tryOut.told !== m.seq && m.ms >= tryOut.files.end + 500) {
    tryOut.told = m.seq;                          // (its input script has played: the machine is the visitor's)
    tell("playing");
  }
  // Inspect the raw frame BEFORE display merging, or unchanged LCD frames would be
  // falsely counted as new hardware updates.
  if (diagnosisActive && m.seq === diagnosisSeq && observeAccuracyFrame(diagnosis, m)) {
    const seconds = diagnosis.observedSeconds;
    diagStatus.textContent = `Measuring… ${Math.min(DIAGNOSTIC_SECONDS, seconds).toFixed(1)}/${DIAGNOSTIC_SECONDS}s`;
    if (seconds >= DIAGNOSTIC_SECONDS) stopDiagnostics();
  }
  // Merge any pending update so a later status-only report cannot erase it.
  if (!m.lcd && frame?.lcd) m.lcd = frame.lcd;
  if (!m.leds && frame?.leds) m.leds = frame.leds;
  if (!m.pads && frame?.pads) m.pads = frame.pads;
  frame = m;
  if (m.lcd || m.leds || m.pads) scheduleDraw();
  perf.add(m);
  const now = performance.now();
  if (!last || now - last.t > 1000) {
    perf.second(m);
    status.textContent = (tryOutError ? `${tryOutError} · ` : "") + (m.halted ? `${fw.name}: the emulated CPU halted` :
      `${fw.name} · ${mhz} MHz · ${accurate ? "Accurate" : "Performance"} · ${perf.speedText()} · load ${perf.loadText()} · ` +
      `${(m.ms / 1000).toFixed(1)} s`);
    // (hover: what the figures mean, the JIT's work in the audio thread, and the audio's health)
    status.title = [`speed: how much faster than real time the emulator runs this firmware on this device ` +
      `(emulated time per time spent emulating; below 1x the sound breaks up)`,
      `load: the share of the audio thread's time the emulator takes (over 100%: the sound breaks up)`,
      m.jit ? `JIT: ${m.jit.regions} regions compiled, ${m.jit.failed} failed${m.jit.error ? ` (${m.jit.error})` : ""}` : "",
      `${m.short} short audio quanta (the firmware's audio fell behind its own clock)`].filter(Boolean).join("\n");
    last = { t: now };
  }
}

async function start(auto = false) {
  startBtn.disabled = true;
  status.textContent = "starting audio…";
  if (!node && !await startAudio(auto)) {         // (auto: the browser wants a click first)
    startBtn.disabled = false;
    status.textContent = "click Start to play the try-out";
    tell("start");
    return;
  }
  const started = await boot();
  if (!started) { startBtn.disabled = false; return; }
  if (midiSel.value) midiOpen();                  // (an input picked on an earlier visit)
  await ac.resume();
  startBtn.hidden = true;
  // The page is viewport-fitted; do not scroll the mobile layout after starting.
}
startBtn.addEventListener("click", () => start());

if (tryOutError) status.textContent = tryOutError;
else if (tryOut) {
  saveMessage("A try-out: it starts from the firmware's own flash and is not saved in this browser");
  start(true);
}

// Persistence controls use the running machine's identity, even while another firmware is selected.
const action = fn => async () => { try { await fn(); } catch (error) { saveFailure(error); } };
document.getElementById("save-now").addEventListener("click", action(async () => {
  if (!running) throw new Error("start a firmware first");
  await flushRunning();
  announce("Emulator state saved in this browser");
}));
document.getElementById("export-save").addEventListener("click", action(async () => {
  if (!running) throw new Error("start a firmware first");
  await flushRunning();
  const record = running.record;
  const url = URL.createObjectURL(new Blob([exportSave(record)], { type: "application/json" }));
  const link = document.createElement("a");
  link.href = url;
  link.download = `${record.name.replace(/[^a-z0-9_-]/gi, '_')}.fm1save`;
  link.click();
  announce("Backup exported");
  setTimeout(() => URL.revokeObjectURL(url), 1000);
}));
document.getElementById("import-save").addEventListener("change", action(async () => {
  const input = document.getElementById("import-save"), file = input.files[0];
  input.value = "";
  if (!file) return;
  if (file.size > 16 << 20) throw new Error("backup too large");
  const record = await importSave(await file.text(), async hash => {
    if (LOCKED_FIRMWARE) {
      const bundled = await packageOf(fw);
      const expectedHash = await packageHash(bundled.bytes);
      return hash === expectedHash ? bundled.bytes : null;
    }
    const kept = memoryRecords.get(hash) || await saveStore.load(hash);
    if (kept) return kept.package;
    if (localFw?.bytes && await packageHash(localFw.bytes) === hash) return localFw.bytes;
    for (const got of packages.values()) if (await packageHash(got.bytes) === hash) return got.bytes;
    return null;
  });
  if (LOCKED_FIRMWARE) {
    const bundled = await packageOf(fw);
    assertAllowedSave(true, await packageHash(bundled.bytes), record);
  }
  if (!confirm(`Import “${record.name}”? This replaces any saved flash for the same firmware package.`)) return;
  await bootQueue;
  await flushRunning();
  let existing;
  try { existing = await saveStore.load(record.hash); } catch (error) { saveFailure(error); }
  record.byo ||= memoryRecords.get(record.hash)?.byo || existing?.byo || false;
  record.revision = memoryRecords.get(record.hash)?.revision || existing?.revision || 0;
  memoryRecords.set(record.hash, record);
  // General edition can restore other firmware packages; RUPTURE remains pinned.
  if (!LOCKED_FIRMWARE) {
    localFw = { id: "local", name: record.name, version: "", bytes: record.package };
    let opt = [...fwSel.options].find(o => o.value === "local");
    if (!opt) fwSel.add(opt = new Option("", "local"), 0);
    opt.text = `${record.name} (your file)`;
    fw = localFw; fwSel.value = "local";
    if (!EMBED) store(DEV.key + "fw", "local");
  }
  bank = null;
  loadedUrl = null;
  showFirmware();
  if (node) await boot({ imported: record });
  else status.textContent = `${record.name} backup ready: click Start`;
  announce("Save imported");
}));
document.getElementById("reset-save").addEventListener("click", action(async () => {
  if (!running || running.entry !== DEV.key + fw.id) throw new Error("start the selected firmware first");
  if (!confirm(`Reset saved settings and patches for “${running.record.name}”? Export a backup first if you want to keep them.`)) return;
  bank = null;
  await boot({ reset: true });
}));
document.getElementById("update-firmware").addEventListener("click", action(async () => {
  if (LOCKED_FIRMWARE) throw new Error("Firmware updates are disabled in the RUPTURE edition");
  if (fw.id === "local" || fw.byo) throw new Error("open a new .fwsc file to update this firmware");
  const selected = fw;
  saveMessage(`Checking ${selected.name}…`);
  const got = await fetchPackage(selected);
  const hash = await packageHash(got.bytes);
  packages.set(selected.id, got);
  noteVersion(selected, got.version);
  if (fw !== selected) return;
  if (running?.record.hash === hash) { saveMessage("Firmware is up to date"); return; }
  if (node) await boot();
  else saveMessage("Firmware ready: click Start");
}));
document.addEventListener("visibilitychange", () => {
  if (document.visibilityState === "hidden" && running) flushRunning().catch(saveFailure);
});

// Closing a tab is best effort: the browser may terminate the worklet before its reply.
// Frequent quiet autosaves remain the durable fallback.
addEventListener("pagehide", () => {
  if (running) flushRunning().catch(saveFailure);
});

// Saved hashes remain accessible after catalog updates and subsequent local-file imports.
const savedVersions = document.getElementById("saved-versions");
document.getElementById("browse-saves").addEventListener("click", action(async () => {
  const records = new Map((await saveStore.list()).map(r => [r.hash, r]));
  for (const [hash, record] of memoryRecords) records.set(hash, record);
  savedVersions.replaceChildren();
  for (const record of records.values()) {
    if (record.device && record.device !== DEVICE) continue;
    savedVersions.add(new Option(`${record.name} · ${record.hash.slice(0, 12)}`, record.hash));
  }
  saveMessage(records.size ? "Choose a saved firmware, then Open saved firmware" : "No saved firmwares yet");
}));
document.getElementById("open-saved-version").addEventListener("click", action(async () => {
  assertFirmwareImportAllowed(LOCKED_FIRMWARE);
  const hash = savedVersions.value;
  const record = memoryRecords.get(hash) || await saveStore.load(hash);
  if (!record) throw new Error("choose a saved firmware first");
  await bootQueue;
  await flushRunning();
  localFw = { id: "local", name: record.name, version: "", bytes: record.package };
  let opt = [...fwSel.options].find(o => o.value === "local");
  if (!opt) fwSel.add(opt = new Option("", "local"), 0);
  opt.text = `${record.name} (saved version)`;
  fw = localFw; fwSel.value = "local";
  if (!EMBED) store(DEV.key + "fw", "local");
  bank = null; loadedUrl = null; showFirmware();
  if (node) await boot();
  else status.textContent = `${record.name} saved version ready: click Start`;
}));
