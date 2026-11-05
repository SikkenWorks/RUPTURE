// The emulator in the audio thread: each 128-frame quantum runs the machine until its I2S
// DMA has produced 128 frames, so the audio device paces the emulation; and also, for an even
// amount of work per quantum, its share of emulated time while the queue holds less than one
// DMA half buffer beyond them (fm1_ahead: half the firmwares hand out 256 or 512 frames at
// once). The page gets the LCD and the LEDs about 30 times a second; keys, knobs and the pot
// come back as messages.
// The WebAssembly JIT's host side, as web/jit.js (a copy: an AudioWorklet does not import
// modules everywhere): compile a hot block's module, install its `run` in the function table.
function jitImports(exports, stats) {
  let next;
  return {
    fm1_jit_compile(ptr, len, hRead, hWrite, hOp) {
      const start = now();
      try {
        const x = exports();
        const bytes = new Uint8Array(x.memory.buffer, ptr, len).slice();
        const table = x.__indirect_function_table;
        // the helpers as function imports, from the table (a module importing the table itself
        // gets a copy of it, grown and updated with every region: 40% of a two-core firmware's
        // time); the table grows by a chunk, the emulator's own instance being its only user
        const env = { memory: x.memory, h_read: table.get(hRead), h_write: table.get(hWrite), h_op: table.get(hOp) };
        const run = new WebAssembly.Instance(new WebAssembly.Module(bytes), { env }).exports.run;
        if (next === undefined || next >= table.length) { next = table.length; table.grow(256); }
        const slot = next++;
        table.set(slot, run);
        stats.regions++;
        stats.bytes += len;
        stats.tableSlots = table.length;
        return slot;
      } catch (e) {
        stats.failed++;
        stats.error = String(e);
        return 0;
      } finally {
        const elapsed = now() - start;
        stats.compileMs += elapsed;
        stats.compileMaxMs = Math.max(stats.compileMaxMs, elapsed);
      }
    },
  };
}

// How much a quantum may emulate, in the emulator's run budget (cpu0's instructions, at most
// one per cycle): while the firmware boots, 20 ms of emulated time, so a boot is quick (there is
// no audio to keep yet); once audio plays, 1.5 quanta, so a firmware that stalls or falls silent
// does not hold the audio thread for 20 ms each quantum. The pace (fm1_ahead) needs one quantum
// when it may run a whole DMA half buffer ahead; fm1_render raises the cap by the rest of the half
// buffer when ahead is less (?ahead=256 with 512-frame halves). Without the pace (?ahead=0) a
// quantum may have to run a whole half buffer's time, so it keeps 20 ms.
const AHEAD_FRAMES = 1024;
// The emulated I2S device always produces 44.1 kHz PCM. Browsers may ignore
// AudioContext({sampleRate:44100}) and open their device at 48 kHz. Without
// persistent rate conversion, a 48-kHz context speeds the FM-1 up by 8.84%.
const FM1_AUDIO_RATE = 44100;

// Continuous, allocation-free (per quantum) linear interpolation. The source
// samples are generated on the *emulated 44.1 kHz clock*, then converted to the
// browser output clock. No changes to firmware, MIDI timing or audio DSP.
// One sample is retained across calls so the boundary never restarts at phase 0.
class Fm1OutputResampler {
  constructor(outputRate) {
    if (!(outputRate > 0)) throw new Error('invalid browser output sample rate');
    this.ratio = FM1_AUDIO_RATE / outputRate;
    this.left = new Float32Array(2048);
    this.right = new Float32Array(2048);
    this.count = 0;
    this.phase = 0;
    this.views = null;
  }
  render(x, out, budget) {
    const n = out[0].length;
    const needed = Math.max(Math.floor(this.phase + Math.max(0, n - 1) * this.ratio) + 2,
      Math.floor(this.phase + n * this.ratio));
    const requested = Math.max(0, needed - this.count);
    if (needed > this.left.length || requested > 1024) throw new Error('audio resampler block exceeds supported size');
    let have = 0;
    if (requested) {
      have = x.fm1_render(requested, budget);
      // fm1_render explicitly clears the unused tail when CPU/audio runs short.
      const buf = x.memory.buffer, lp = x.fm1_out_l(), rp = x.fm1_out_r();
      if (!this.views || this.views.buffer !== buf || this.views.leftOffset !== lp || this.views.rightOffset !== rp) {
        this.views = { buffer: buf, leftOffset: lp, rightOffset: rp,
          left: new Float32Array(buf, lp, 1024), right: new Float32Array(buf, rp, 1024) };
      }
      this.left.set(this.views.left.subarray(0, requested), this.count);
      this.right.set(this.views.right.subarray(0, requested), this.count);
      this.count += requested;
    }
    let phase = this.phase;
    const l = out[0], r = out[1];
    for (let i = 0; i < n; i++) {
      const j = Math.floor(phase), fraction = phase - j;
      l[i] = this.left[j] + (this.left[j + 1] - this.left[j]) * fraction;
      if (r) r[i] = this.right[j] + (this.right[j + 1] - this.right[j]) * fraction;
      phase += this.ratio;
    }
    const consumed = Math.floor(phase);
    if (consumed) {
      this.left.copyWithin(0, consumed, this.count);
      this.right.copyWithin(0, consumed, this.count);
      this.count -= consumed;
    }
    this.phase = phase - consumed;
    return { have, requested };
  }
}
function budgets(mhz, frames) {
  return { boot: mhz * 1e6 * 0.02, play: mhz * 1e6 * 1.5 * frames / sampleRate };
}

// the wall clock, for how long each quantum's emulation takes (where an AudioWorklet has no
// performance.now(), Date.now(): whole milliseconds, which still average out over a second)
const now = globalThis.performance?.now ? () => performance.now() : () => Date.now();

class Fm1 extends AudioWorkletProcessor {
  constructor() {
    super();
    this.x = null;
    this.calls = 0;
    this.bootToken = 0;
    this.snapshots = [];
    this.monitor = false;
    this.diagnostics = { peak: 0, clipped: 0, sumSquares: 0, audioSamples: 0, quantumMs: [] };
    this.port.onmessageerror = () => this.port.postMessage({ type: "error", message: "a message could not be read" });
    this.port.onmessage = async (e) => {
      const m = e.data;
      if (m.type === "boot") {
        const token = ++this.bootToken;
        // Keep the previous (paused) instance until the replacement boots successfully.
        try {
          await this.boot(m, token);
        } catch (err) {
          if (token !== this.bootToken) return;
          this.paused = false;
          this.port.postMessage({ type: "error", seq: m.seq, message: `boot: ${err}` });
        }
      } else if (m.type === "monitor") {
        this.monitor = !!m.enabled && !!this.x;
        this.diagnostics = { peak: 0, clipped: 0, sumSquares: 0, audioSamples: 0, quantumMs: [] };
      } else if (m.type === "cancel-boot") {
        ++this.bootToken;
        this.x = null;
        this.snapshots = [];
      } else if (m.type === "snapshot" && !this.x) {
        this.port.postMessage({ type: "snapshot", seq: m.seq, request: m.request, unavailable: true });
      } else if (this.x) {
        if (m.type === "snapshot") {
          if (m.seq !== this.seq) return;
          this.snapshots.push({ ...m, started: now() });
          this.flushSnapshots();
        } else if (m.type === "cancel-snapshot") this.snapshots = this.snapshots.filter(r => r.request !== m.request);
        else if (m.type === "resume") this.paused = false;
        else if (m.type === "key") this.x.fm1_key(m.id, m.down ? 1 : 0);
        else if (m.type === "enc") this.x.fm1_enc(m.enc, m.clicks);
        else if (m.type === "pot") this.x.fm1_pot(m.value);
        else if (m.type === "midi") for (const b of m.bytes) this.x.fm1_midi(b);
        else if (m.type === "analog") this.x.fm1_analog(m.idx, m.value);   // SMK-37: pads, faders, wheels
        else if (m.type === "vel") this.x.fm1_velocity(m.us);             // SMK-37: the next keys' velocity
      }
    };
  }

  async boot(m, token) {
    // the module arrives as bytes: a compiled WebAssembly.Module does not cross into the
    // AudioWorklet's scope (Chrome delivers it as a messageerror). Compiled once, here: on a
    // phone compiling it again on every firmware change takes a noticeable while
    if (!this.module) this.module = await WebAssembly.compile(m.wasm);
    let x;
    this.jit = { regions: 0, failed: 0, error: "", bytes: 0, tableSlots: 0, compileMs: 0, compileMaxMs: 0 };
    x = (await WebAssembly.instantiate(this.module, { env: jitImports(() => x, this.jit) })).exports;
    if (token !== this.bootToken) return;
    // Explicit feature check: never pretend an older self-hosted WASM is hardware-accurate.
    if (m.accurate && !x.fm1_strict) throw new Error("Hardware Accuracy needs the V2 WebAssembly build");
    x.fm1_strict?.(m.accurate ? 1 : 0);
    x.fm1_lcd_timed?.(m.accurate ? 1 : 0);
    // Bound synchronous WASM compilation per quantum. After audio starts only two
    // hot blocks can be compiled per callback, limiting real-time audio spikes.
    x.fm1_jit(m.jit === false || m.accurate ? 0 : 1, 6);
    const ahead = x.fm1_ahead ? m.ahead ?? AHEAD_FRAMES : 0;
    x.fm1_ahead?.(ahead);               // (?ahead=0: each quantum runs for its own frames only)
    // the whole update package (.fwsc): the machine unpacks it, gets the flash chip and the
    // boot parameters from it (what M-VAVE's own firmware needs; the others boot the same)
    const copy = (buf) => {
      const p = x.fm1_alloc(buf.byteLength);
      new Uint8Array(x.memory.buffer, p, buf.byteLength).set(new Uint8Array(buf));
      return p;
    };
    const pkg = copy(m.package);
    if (x.fm1_boot_fwsc(pkg, m.package.byteLength, 0, 0, m.mhz) !== 0) {
      throw new Error("not an FM-1 update package (or a DX7 bank it cannot use)");
    }
    if (m.sectors?.byteLength && x.fm1_flash_restore(copy(m.sectors), m.sectors.byteLength) !== 0) {
      throw new Error("invalid saved flash sectors");
    }
    if (m.syx && x.fm1_load_syx(copy(m.syx), m.syx.byteLength) === 1) {
      throw new Error("this firmware cannot load that DX7 bank");
    }
    // a try-out's input script (web/tryout.js): UTF-8 bytes, played on the emulated clock from now
    if (m.script?.byteLength && (!x.fm1_bench_script || x.fm1_bench_script(copy(m.script), m.script.byteLength) !== 0)) {
      throw new Error("the input script did not load");
    }
    this.x = x;                         // (a reboot replaces the whole instance)
    this.monitor = false;
    this.diagnostics = { peak: 0, clipped: 0, sumSquares: 0, audioSamples: 0, quantumMs: [] };
    this.mhz = m.mhz;
    this.accurate = !!m.accurate;
    this.jitEnabled = m.jit !== false && !m.accurate;
    this.ahead = ahead;
    this.budget = budgets(m.mhz * (this.accurate ? 8 : 1), 128).boot;
    this.playing = false;               // the firmware's audio has started (the budget is capped)
    this.lastTiming = null;
    this.views = null;
    this.outputResampler = sampleRate !== FM1_AUDIO_RATE ? new Fm1OutputResampler(sampleRate) : null;
    this.lcdRevision = null;
    this.lastLeds = null;
    this.lastPads = null;
    this.reportFrames = 0;
    this.reportCalls = 0;
    this.short = 0;
    this.calls = 0;
    this.busy = 0;                      // ms spent emulating since the last frame message
    this.worst = 0;                     // the longest quantum among them (once audio plays)
    this.timedMs = 0;                   // the emulated ms when the timed interval began
    this.timedIdle = 0;                 // and the idle ms skipped by then
    this.board = x.fm1_board();         // 0 the FM-1, 1 the SMK-37 (the package says which)
    this.seq = m.seq;
    this.paused = false;
    this.snapshots = [];
    this.flashRevision = x.fm1_flash_revision();
    this.observedRevision = this.flashRevision;
    this.flashChangedAt = this.lastSnapshot = now();
    const base = new Uint8Array(x.memory.buffer, x.fm1_flash_base(), x.fm1_flash_len()).slice();
    const flash = new Uint8Array(x.memory.buffer, x.fm1_flash(), x.fm1_flash_len()).slice();
    this.port.postMessage({ type: "booted", seq: m.seq, board: this.board, revision: this.flashRevision, base, flash }, [base.buffer, flash.buffer]);
  }

  observeFlash() {
    const revision = this.x.fm1_flash_revision();
    if (revision !== this.observedRevision) {
      this.observedRevision = revision;
      this.flashChangedAt = now();
      this.port.postMessage({ type: "flash-changed", seq: this.seq, revision });
    }
    return revision;
  }

  flushSnapshots() {
    const x = this.x;
    if (!x) return;
    this.observeFlash();
    const quiet = now() - this.flashChangedAt >= 250 || x.fm1_halted();
    const busy = x.fm1_flash_busy();
    for (const request of this.snapshots.splice(0)) {
      if (busy || !quiet) {
        if (request.request !== undefined && now() - request.started >= 1500) {
          // A constantly writing firmware must not prevent switching away. Keep its
          // last stable save instead of persisting a possibly half-written sector.
          if (request.pause) this.paused = true;
          this.port.postMessage({ type: "snapshot", seq: this.seq, request: request.request,
            unavailable: true, error: request.pause ? null : "flash did not become quiet; try saving again" });
        } else this.snapshots.push(request);
        continue;
      }
      const flash = new Uint8Array(x.memory.buffer, x.fm1_flash(), x.fm1_flash_len()).slice();
      this.flashRevision = x.fm1_flash_revision();
      this.observedRevision = this.flashRevision;
      this.lastSnapshot = now();
      if (request.pause) this.paused = true;
      this.port.postMessage({ type: "snapshot", seq: this.seq, request: request.request,
        revision: this.flashRevision, flash }, [flash.buffer]);
    }
  }

  process(_inputs, outputs) {
    const out = outputs[0];
    const x = this.x;
    if (!x || this.paused) return true;
    const n = out[0].length;
    const playing = this.playing;       // (the worst quantum: of those once audio played)
    const t0 = now();
    let have, requested = n;
    try {
      if (this.outputResampler) {
        const result = this.outputResampler.render(x, out, this.budget);
        have = result.have;
        requested = result.requested;
      } else {
        have = x.fm1_render(n, this.budget);
      }
    } catch (err) {                       // a trap in the emulator: stop and say so
      this.x = null;
      for (const request of this.snapshots.splice(0)) {
        this.port.postMessage({ type: "snapshot", seq: this.seq, request: request.request, unavailable: true });
      }
      this.port.postMessage({ type: "error", seq: this.seq, fatal: true, message: `render: ${err}` });
      return true;
    }
    if (!this.outputResampler) {
      const buffer = x.memory.buffer, lp = x.fm1_out_l(), rp = x.fm1_out_r();
      if (!this.views || this.views.l.buffer !== buffer || this.views.l.length !== n ||
          this.views.l.byteOffset !== lp || this.views.r.byteOffset !== rp) {
        this.views = { l: new Float32Array(buffer, lp, n), r: new Float32Array(buffer, rp, n) };
      }
      out[0].set(this.views.l);
      if (out[1]) out[1].set(this.views.r);
    }
    // Diagnostics are opt-in. Never copy or transmit actual audio samples, only
    // aggregate levels and observed callback durations while the test runs.
    if (this.monitor && (this.playing || have > 0)) {
      const d = this.diagnostics;
      const l = out[0], r = out[1];
      // Measure the actual browser output, including resampling and short-frame
      // silence, rather than only a partial 44.1 kHz source buffer.
      for (let i = 0; i < n; i++) {
        const a = l[i], b = r?.[i];
        if (Number.isFinite(a)) {
          d.peak = Math.max(d.peak, Math.abs(a));
          d.clipped += Math.abs(a) >= 1 ? 1 : 0;
          d.sumSquares += a * a;
          d.audioSamples++;
        }
        if (out[1] && Number.isFinite(b)) {
          d.peak = Math.max(d.peak, Math.abs(b));
          d.clipped += Math.abs(b) >= 1 ? 1 : 0;
          d.sumSquares += b * b;
          d.audioSamples++;
        }
      }
    }
    if (have > 0 && !this.playing) {
      this.playing = true;
      if (this.ahead > 0) this.budget = budgets(this.mhz * (this.accurate ? 8 : 1), n).play;
      if (this.jitEnabled) x.fm1_jit(1, 2);
    }
    this.short = (this.short || 0) + (have < requested ? 1 : 0);
    this.reportFrames += n;
    this.reportCalls++;
    if (this.reportFrames >= sampleRate / 30) {
      const flashRevision = this.observeFlash();
      if (flashRevision !== this.flashRevision && now() - this.lastSnapshot >= 1000 &&
          now() - this.flashChangedAt >= 250 && !this.snapshots.length) {
        this.snapshots.push({});
      }
      this.flushSnapshots();
      const revision = x.fm1_lcd_revision();
      const lcd = revision !== this.lcdRevision ? new Uint16Array(x.memory.buffer, x.fm1_lcd(), 240 * 240).slice() : null;
      this.lcdRevision = revision;
      const ledsView = new Float32Array(x.memory.buffer, x.fm1_leds(), 41);
      // Don't allocate/post unchanged LEDs. The main thread has the previous levels.
      let leds = null;
      if (!this.lastLeds || ledsView.some((v, i) => v !== this.lastLeds[i])) {
        leds = ledsView.slice();
        this.lastLeds = leds;
      }
      const padsView = this.board ? new Uint32Array(x.memory.buffer, x.fm1_pads(), 16) : null;
      let pads = null;
      if (padsView && (!this.lastPads || padsView.some((v, i) => v !== this.lastPads[i]))) {
        pads = padsView.slice();
        this.lastPads = pads;
      }
      const transfer = [];
      if (leds) transfer.push(leds.buffer);
      if (lcd) transfer.push(lcd.buffer);
      if (pads) transfer.push(pads.buffer);
      // Report the preceding interval: its final postMessage is included in its timing.
      const ms = x.fm1_ms(), idle = x.fm1_idle_ms?.() ?? 0;
      const diagnostics = this.monitor ? this.diagnostics : null;
      this.port.postMessage({ type: "frame", seq: this.seq, lcd, leds, ms,
        short: this.short, halted: x.fm1_halted(), jit: this.jit, pads, playing: this.playing,
        busy: this.lastTiming?.busy ?? 0, worst: this.lastTiming?.worst ?? 0,
        emulated: this.lastTiming?.emulated ?? 0, idle: this.lastTiming?.idle ?? 0,
        quanta: this.lastTiming?.quanta ?? 0, frames: this.lastTiming?.frames ?? n,
        engineRate: FM1_AUDIO_RATE, outputRate: sampleRate, resampled: !!this.outputResampler, diagnostics }, transfer);
      this.diagnostics = { peak: 0, clipped: 0, sumSquares: 0, audioSamples: 0, quantumMs: [] };
      const took = now() - t0;
      if (this.monitor && this.diagnostics.quantumMs.length < 32) this.diagnostics.quantumMs.push(took);
      // busy: ms of wall clock the interval's quanta took; emulated: ms of emulated time they ran,
      // idle: of which idle skipping jumped over (cost nothing)
      this.lastTiming = { busy: this.busy + took, worst: playing ? Math.max(this.worst, took) : this.worst,
        emulated: ms - this.timedMs, idle: idle - this.timedIdle,
        quanta: this.reportCalls, frames: this.reportFrames / this.reportCalls };
      this.timedMs = ms;
      this.timedIdle = idle;
      this.busy = 0;
      this.worst = 0;
      this.reportFrames = 0;
      this.reportCalls = 0;
    } else {
      const took = now() - t0;
      if (this.monitor && this.diagnostics.quantumMs.length < 32) this.diagnostics.quantumMs.push(took);
      this.busy += took;
      if (playing) this.worst = Math.max(this.worst, took);
    }
    return true;
  }
}
registerProcessor("fm1", Fm1);
