# FM-1 Emulator V2.3 — Reproducible hardware comparison

The FM-1 Emulator is based on **Joakim Eriksson's** work:
https://github.com/joakimeriksson/fm1-emu (MIT license).

## V2.3: browser sample-rate conversion

V2.3 preserves the FM-1's native 44.1 kHz audio timeline even if the browser's
output AudioContext uses another rate. Diagnostics records both rates. When
comparing WAV files, export the emulator's **44.1 kHz offline benchmark** and
the physical recording at 44.1 kHz as before; do not treat WebAudio output
clock conversion as a correction to the FM-1's CPU clock or MIDI/USB timing.
Try the Fast/Balanced/Stable **Response** presets on your actual phone and
compare Diagnostics underruns and input response. The ahead buffer values are
not equal to the total round-trip latency.

## What can be tested

1. **Browser performance:** use the toolbar's **Diagnostics** button. Start the emulator,
   click **Run 10-second test**, and save the JSON. It contains callback load and
   percentiles, reported short audio quanta, observed LCD/LED changes, output peak
   and RMS. **No audio or MIDI notes are recorded by Diagnostics**.
2. **Differential audio:** send the same notes and use the same firmware/patch on a
   real FM-1 and in the emulation. Compare two 16-bit PCM WAV files with the
   offline Python script.
3. **Deterministic timing:** the command-line WebAssembly runner can replay the
   timestamped actions on the emulated clock. `FM1_STRICT=1` enables the same
   Hardware Accuracy mode exposed in the browser. It is slower and not guaranteed
   to keep up with real-time playback on every computer.

## Reproducible MIDI stimulus

The supplied `tools/reference-notes.txt` contains exactly 14 MIDI note-on and
note-off messages over 6 seconds. `tools/reference-notes.mid` contains those
*same events* at 120 BPM as a Type-0 MIDI file. To generate another test:

```bash
python3 tools/script_to_midi.py tools/reference-notes.txt tools/reference-notes.mid
```

The script accepts **only** timestamped `ms:mHEX` MIDI channel messages. A
firmware benchmark can additionally accept key and encoder actions, but these
cannot automatically be replicated by playing a MIDI file on real hardware.

## Run the emulator against a particular firmware

Build a local WASM engine first (`bash web/build.sh _site`) or use the GitHub
Actions build. From the project source root:

**Linux/macOS**

```bash
export FM1_FWSC=/path/to/exact-firmware.fwsc
export FM1_BENCH_KEYS="$(cat tools/reference-notes.txt)"
export FM1_STRICT=1
export FM1_BENCH_JSON=emulator-accurate.json
node web/bench.mjs 240 8 emulator-accurate.wav
```

**Windows PowerShell**

```powershell
$env:FM1_FWSC = 'C:\path\to\exact-firmware.fwsc'
$env:FM1_BENCH_KEYS = (Get-Content tools\reference-notes.txt -Raw).Trim()
$env:FM1_STRICT = '1'
$env:FM1_BENCH_JSON = 'emulator-accurate.json'
node web/bench.mjs 240 8 emulator-accurate.wav
```

`web/bench.mjs` looks for the WASM at
`target/wasm/wasm32-unknown-unknown/release/fm1_web.wasm` unless
`FM1_WASM` is set to a different path. The output is true 44.1 kHz,
16-bit stereo PCM WAV. Without `FM1_STRICT=1`, the JIT-based Performance
mode is used. Benchmark runs are not real-time playback: they can take as long
as needed to complete the emulator's requested timeline.

## V2.2: Compare directly inside the PWA

Open the PWA toolbar's **Diagnostics** dialog and scroll to **Physical FM-1 audio
comparison**. Import two 16-bit mono/stereo PCM WAV files with the same sample rate:
first the analog FM-1 recording, then the emulator recording. Select the start offset
(usually 0.5 seconds) and maximum anticipated lag (usually 250 ms), and click
**Compare WAV recordings**. Download only the numerical JSON result; the recorded
audio remains on your device.

The browser tool detects onset envelopes before waveform alignment to reduce the
ambiguity of repetitive oscillator cycles. It reports the alignment of the *files*,
not pure audio chip output latency. **Do not automatically use the reported offset to
change emulator CPU clock, LCD/SPI timing or MIDI scheduler.** To claim real timing
accuracy, also synchronize the captures' trigger timestamps and account for the
physical audio-interface chain.

The older `tools/compare_audio.py` comparison CLI remains available below; its
algorithm and analysis window are not identical to the PWA implementation, so minor
numerical differences are expected.

## Record the physical FM-1

- Install exactly the *same* firmware image and select the same saved patch.
- Keep external/analog FX and audio processing off. Match level and trim as
  well as possible. The FM-1's analog output circuitry will introduce noise,
  latency and distortion absent from the ideal emulated output.
- Send `tools/reference-notes.mid` from a DAW over a MIDI connection **to the
  FM-1**. Use a direct hardware MIDI connection if you wish to compare the
  emulator's DIN-UART input path; USB-MIDI may take a different path inside the
  firmware. Record the audio output through an audio interface.
- Export that recording as **16-bit PCM WAV at 44100 Hz**. Ideally capture at
  least 8 seconds, including at least 500 ms before the first note. Do not
  normalize or add reverb in the DAW.
- For a useful lag estimate, line up the start of both files approximately.
  The comparison script searches only within the allowed lag window.

Compare in PowerShell or a terminal:

```bash
python3 tools/compare_audio.py fm1-physical.wav emulator-accurate.wav --max-lag-ms 250 --json comparison.json
```

The report shows **lag in milliseconds, correlation, level difference in dB,
polarity and residual error**. No threshold is hardcoded as pass/fail: a true
hardware reference, the I/O method and measurement noise all matter.

## Minimum release checks

- Use the same exact `.fwsc` file (and package hash) throughout comparisons.
- Repeat with three different patches and dry/wet FX configurations.
- Check the idle screen, full-screen LCD redraws, encoders and note transitions
  on the physical hardware and the emulator separately.
- Run a 10-second browser report in both Performance and Hardware Accuracy
  modes, comparing **load and underruns**, not treating speed as accuracy.
- If the engine crashes, logs an invalid package or runs out of audio frames,
  preserve the report and mark the run as invalid; do not invent an audio match.

**Scope:** no live physical FM-1, MIDI cable, USB audio capture, microphone
or audio interface was available for automatic validation in the build environment.
This package provides the reproducible tooling, not a claim of validated parity.
