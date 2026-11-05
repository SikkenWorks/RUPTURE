# FM-1 Emulator V2.3 — Hardware Calibration PWA

**Original FM-1 emulator by Joakim Eriksson** · https://github.com/joakimeriksson/fm1-emu  
Original MIT copyright © 2026 Joakim Eriksson · complete license in `LICENSE`.

This is a full-screen, phone-friendly website/PWA derived from Joakim's FM-1 emulator. The emulator core and firmware/save protocol remain his. This version modifies the browser panel renderer, the AudioWorklet's JIT policy and update messages, and the Rust WebAssembly interface to expose a hardware-accurate mode based on the upstream native `FM1_STRICT` implementation.

## V2.3: corrected audio-clock handling and response setting

FM-1 firmware emits 44,100 PCM samples per second. Some browsers ignore the
requested AudioContext sample rate and deliver 48,000 frames per second instead.
**V2.3 continuously converts** native FM-1 samples to the browser's reported
output rate, so notes no longer play too high/fast in those browsers. At 44.1
kHz the conversion is bypassed and samples are copied unchanged. The conversion
runs inside the AudioWorklet; it is not an FM-1 DSP or firmware modification.

The new **Response** toolbar setting controls how far the emulated audio engine
runs ahead of input:

| Response | FM-1 source frames of ahead | Approximate queue equivalent |
|---|---:|---:|
| Fast | 256 | 5.8 ms |
| Balanced | 512 | 11.6 ms |
| Stable (default) | 1024 | 23.2 ms |

This queue is **not** the device's total latency; MIDI transfer, browser,
interface and speaker/output buffering are additional. Fast can reduce input
lag at the cost of increased underruns. The default remains Stable for
backward-compatible behavior. Selecting a response restarts the emulated
firmware after preserving its flash state.

**Diagnostics** now identifies the 44,100 Hz emulation clock and the actual
browser output sample rate, and marks when resampling is active. Neither these
metrics nor a WAV comparison alone proves that the emulator matches real FM-1
hardware timing. Accurate chip/peripheral calibration still requires reference
captures from a physical unit using the same firmware, patch and MIDI input.

## What V2.2 adds

- **Physical FM-1 Audio Comparison** under the Diagnostics panel. Import a physical FM-1
  recording and an emulator WAV locally in your browser. The two mono/stereo PCM16 files
  must use the same sample rate; 44.1 kHz is recommended.
- Compares timing alignment, correlation, polarity, volume difference, peaks and residual
  error, with an exportable JSON result **without the raw recordings**.
- Identical MIDI notes and the same firmware/patch are essential. This is a tool for
  collecting real measurements, not a claim that the emulator is cycle-accurate.
- Keeps the emulator rendering, DSP, saves, custom shortcuts, and MIDI unchanged.

## What V2.1 adds

- The **Diagnostics** toolbar button opens a compact 10-second measurement tool.
- The audio thread samples callback timings and aggregate audio levels **only while measuring**. No microphone recording, played notes or raw audio is exported.
- Download a JSON diagnostic report containing the exact firmware SHA-256, CPU clock, mode, callback p50/p95/p99, dropouts, peak/RMS and observed LCD changes. This is **emulator performance**, not proof of equivalence with FM-1 hardware.
- The full source ZIP contains `tools/compare_audio.py`, `tools/script_to_midi.py` and reproducible MIDI test files. Use these with an actual FM-1 and the CLI benchmark to quantify how closely audio matches. See `tools/README-VALIDATION.md` in the GitHub source package.
- The CLI benchmark now supports `FM1_STRICT=1` and WAV export for offline comparisons.
- Updated PWA cache and added automated browser logic tests + Python tests.

## What's new in V2.0

- **Event-driven panel rendering:** the 1280×820 panel is drawn only when the LCD, LEDs, pads or keys change. A short animation loop runs while a rotary control turns, then stops. No permanent 60 FPS full-panel redraw.
- **Cached ImageData:** when the WebAssembly memory/pixel-buffer pointers are unchanged, the same `ImageData` wrapper is reused. It is recreated after a WASM memory growth.
- **Less worklet messaging:** the LCD is still revision-gated, and now identical LED/pad buffers are omitted from frame messages. The page merges pending updates correctly.
- **Bounded JIT compilation:** at most six new JIT blocks per quantum at startup, then two while audio is playing. This limits compilation bursts but does not eliminate all stutters.
- **Hardware Accuracy:** a separate mode using the strict interpreter's measured instruction costs, RAM alignment checks, 8 ticks per CPU cycle, XIP flash-cache timing, and timed LCD transfers. The mode defaults to 240 MHz when selected, but the clock remains adjustable. Switching modes reboots the emulated firmware while preserving saved flash.
- **Mobile-first PWA:** full-screen stage, horizontal toolbar, touch controls, keyboard shortcut editor, save backup import/export, Web MIDI input when supported, and user's supplied favicon/app icons.

## Two modes: use the right one

| Mode | Intended use | Technical behavior |
| --- | --- | --- |
| **Performance** | Play and design sounds, especially on phones | WebAssembly JIT, instant LCD transfers, shorter audio-thread compile bursts. Default 100 MHz on first use for slower hardware; clock can be changed. |
| **Hardware Accuracy** | Inspect firmware timing / compare with a real FM-1 | Interpreter with native `FM1_STRICT` costs, XIP cache, alignment traps, LCD/SPI timing, default 240 MHz. Likely slower and may cause audio underruns on many devices. |

**Not a cycle-perfect emulator:** timing values are modeled or inferred from hardware work. Audio/peripheral behavior still needs differential testing on a real FM-1. No measured performance gain or hardware parity is claimed without tests on target devices.

## Publish on GitHub Pages (recommended)

1. Upload the **complete contents of the SOURCE-GITHUB ZIP** into a new GitHub repository.
2. Repository **Settings → Pages → Build and deployment → GitHub Actions**.
3. Push to `main` or `master`. `.github/workflows/pages.yml` runs JavaScript tests, builds the new Rust WebAssembly engine, verifies `fm1_web.wasm`, and deploys a self-hosted static PWA.
4. Load the published site over HTTPS, import your own `.fwsc` update package and tap Start.

**For your own web server:** after the GitHub Action completes, download its published site artifact and upload the files to your server, or build locally using `rustup target add wasm32-unknown-unknown && bash web/build.sh`. Keep `index.html` and `fm1_web.wasm` in the same folder. HTTPS or localhost is required for PWA and Web MIDI permissions.

## The small WEBSITE-UPLOAD ZIP: important limitation

The supplied upstream source did **not** contain a prebuilt `fm1_web.wasm`, and this environment cannot compile Rust. This small ZIP includes everything except a compiled emulator core. It tries the original author's online WASM as a fallback, if cross-origin access permits. **Hardware Accuracy is automatically disabled with a WASM build that lacks `fm1_strict`.** Do not treat the small ZIP as a complete standalone/offline V2.3 build. Use the GitHub Action build for both modes and offline capability.

## Controls, persistence and credits

- Import your own `.fwsc` locally; no update is sent to any physical FM-1. No bundled firmware and no public catalog UI.
- Settings: emulator mode, MHz, audio response, save, export/import `.fm1save`, MIDI device/channel and customizable keyboard shortcuts.
- The emulator's firmware flash saves automatically in IndexedDB when writes settle. Use the emulated firmware's own SAVE action to store its RAM edits, and export backups before clearing browser data.
- Shortcuts are editable in the Shortcuts dialog, including note keys and encoder moves. Text selection and right click are disabled via JavaScript (not a security mechanism).
- On iOS, Web MIDI is generally unavailable: play via touch or attached keyboard. Audio requires a gesture. Installing the PWA does not override these browser limitations.

**Source credit:** Joakim Eriksson, [FM-1 Emulator](https://github.com/joakimeriksson/fm1-emu), MIT. Original reverse-engineering community credits remain in `UPSTREAM_README.md`. This derivative is unaffiliated with M-VAVE.

## Tests

Run `node --test web/calibration.test.mjs web/accuracy.test.mjs web/audio-file.test.mjs web/render-scheduler.test.mjs web/main.test.mjs web/fwsc.test.mjs web/persistence.test.mjs web/jit.test.mjs web/tryout.test.mjs web/worklet.test.mjs` from the source root. JavaScript tests mock the WASM/device; they are not firmware boot tests. Also run `python3 -m unittest discover -s tools/tests -v`. The updated Rust source still requires a real `cargo build` and test on browser + FM-1 hardware.
