# RUPTURE V1.81 — Rapid-Note DMA Headroom Audit

**Date:** 2026-11-04. Base: RUPTURE V1.80 complete source. **Status: native FM1 build + focused host tests passed; hardware validation required.** The user reports that DMA LATE and GUARD counters increase when rapidly playing notes on a real FM1. They report that the V1.80 reverb sound is now satisfactory. No reverb changes were authorized or made.

## Source-level findings

1. RUPTURE is strictly monophonic, but `native_rates()` and organic/LIVING control-rate housekeeping still calculated eight-slot information on note changes and every 32 audio samples. The DSP voice renderer itself was already mono. V1.81 handles only the one active voice in RUPTURE, leaving the ETCH eight-voice implementation intact under the non-RUPTURE build.
2. `mono_perf_tick()` rewrote all held-note stack records on **every audio sample** even if no timed sequencer note expired. V1.81 performs the same gate countdown in place, compacts only on an actual gate expiration, and calls `mono_resume()` at exactly the same time as V1.80. This is an optimization of housekeeping, not a different voice-stealing/legato policy.
3. `bringup.c` treated every Note ON/OFF change as an `urgent` reason to perform a full LCD repaint from the main loop, bypassing the previously added UI pressure budget. Under dense note changes this can generate repeated LCD/SPI traffic. V1.81 records note bursts and merges purely visual repaints to a minimum 75 ms interval during bursts, increased to 100 ms under recent ISR wall-time pressure. Ordinary page/menu controls remain urgent. A final visual frame is permitted when a burst subsides; note input and full-rate PCM are unchanged.

The LCD bus contention explanation is **consistent with**, but not hardware-proven by, the reported screensaver observation. Other contributors (USB MIDI interrupts, main-loop bus arbitration, I2S/DMA servicing, power stability) are not excluded.

## Executed tests

| Check | Result |
|---|---|
| JieLi pi32v2 FM1 crossbuild with storage, OTA, USB audio 44.1/48k and multi-project | PASS |
| FM1 application size | 347,876 / 581,564 bytes, free 233,688 |
| Repeated crossbuild reproducibility | PASS, byte-identical SHA-256 |
| Native rapid-note host PCM comparison (V1.80 vs V1.81) | **PASS, exact identical 64-bit PCM digest `4f603683ade7da59`** |
| Host PCM scenario | 512,000 stereo frames, repeated MIDI note-on/off, fast sub-drift sweeps, 16-step sequencer, swing, multi-step gates and MIDI overlap |
| New LCD note-burst unit test | PASS: normal notes, rapid note coalescing, pressure budget, user-command bypass, burst timeout |
| Sustained Note ON/OFF and circular sequence shifting | PASS, existing V1.74 tests |
| Downbeat count-in and timestamp-aware quantization | PASS, existing V1.76 tests |
| 3 distinct reverb types, wet 100%, storage roundtrip | PASS, existing V1.78 and V1.80 tests |
| All reverb DSP source (`native_fx.c`) | SHA-256 **byte-identical to V1.80** |
| OTA source (`ota.c`) and trusted OTA loader | SHA-256 **byte-identical to V1.80** |
| Protected 16 KiB boot header | SHA-256 **byte-identical to V1.80** |
| Browser WebInstaller embedded package / hash / source release | PASS, site + installer tests |
| Physical FM1 DMA LATE/GUARD after update | **NOT TESTED** |
| Worst-case FM1 hardware cycles, minimum slack or audio monitoring | **NOT TESTED** |
| Full historical regressions | **NOT FULLY PASSING**: V1.79 pre-reverb-change FX parity test still expects V1.78-era FX PCM. V1.80 deliberately replaced SPRING/ROOM and thus fails that obsolete comparison. Prior unrelated UI pixel-regression was also open. |

## Runtime and audio limitations

- Equal PCM from the tested host sequence means this optimization did not alter the resulting samples in those specific cases; it does not prove the absence of every rare branch or timing fault.
- No recorded physical FM1 cycle improvement is available. Host wall-clock benchmarks vary with load and are **not** valid FM1 CPU/DMA measurements. We do not claim the actual DAC will be crackle-free or LATE/GUARD will remain zero in every possible setup.
- Full flash ROM restore on this particular FM1, and its stock V15 downgrade, are not newly validated by this release. The firmware package retains the existing OTA identity `FM-1_968`, original loader and 16 KiB bootheader; normal recovery precautions still apply.
- The LCD may show fewer intermediate animation frames during fast playing, but no animations, sequencer functions, synth features, storage data or reverb audio were removed. There is no new reverb memory or additional audio voice.

## Necessary physical acceptance test

1. Back up projects; verify access to the currently working Pico recovery method. Power-cycle the FM1 so cumulative counters begin from a known baseline.
2. Open GLO → AUDIO DIAGNOSTICS and record **DSP MAX, MIN SLACK, DMA LATE and GUARD** before pressing any keys.
3. With the same preset that previously caused counter growth, repeatedly play rapidly (including same-note retriggers and alternating pitches). Use both SUBs with DRIFT and the full ROOM/HALL/SPRING FX load; enable USB MIDI/audio if that's part of the actual workflow.
4. Compare the **difference** in DMA LATE and GUARD counts. The required acceptance result for live/studio confidence is **zero new events**, stable positive MIN SLACK and no audible crackling after a sustained session. If LATE/GUARD increase, the issue is **not solved** on hardware and requires a fresh on-device timing trace/audit rather than further guesses.

**Conclusion:** V1.81 is a measured code-path and LCD-contention mitigation with native binary/OTA integrity and host audio parity. It is **not yet a hardware-certified real-time fix**.
