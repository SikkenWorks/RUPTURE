# RUPTURE V1.95: realtime and sound-preservation audit

## Audio PCM parity

- V1.94 unmodified DSP was compiled separately to generate immutable PCM oracle hashes.
- All ten FX types, one instance and eight separate instances each, render **exactly the same** 35,000 stereo PCM frames as V1.94 (20 scenarios, 700,000 total frames).
- Each duplicate slot has different control values. The tests toggle the fourth slot and reorder slots while running.
- An additional 36,000-frame oracle verifies old (shared-control) project routing and new independent FX with live hidden-modulation on/off and changes to chorus/delay/reverb controls. Both paths are byte-identical to V1.94.
- A further 44,000-frame oracle verifies **three simultaneous WAHs whose speed/resonance knobs change during playback**, including slot movement, enable/bypass, effect replacement and re-enabling. Output remains bit-identical to V1.94.
- Existing V1.94 tests also compare all ten unique FX types against the older DSP.

## Host-only CPU microbenchmark

AMD64/Linux desktop compiler `cc -std=c11 -O3 -DRUPTURE_BUILD`; 300,000 PCM frames per scenario, ten alternated baseline/new runs. Time is the **median** nanoseconds per PCM frame. These numbers are **not** pi32v2 FM1 CPU cycle counts, and scheduler/frequency variation remains measurable.

| Scenario | V1.94 ns/frame | V1.95 ns/frame | Change |
|---|---:|---:|---:|
| 8 empty | 15.96 | 14.52 | -9.0% |
| 1 reverb | 55.30 | 51.89 | -6.2% |
| 8 reverbs | 104.96 | 95.19 | -9.3% |
| 1 delay | 40.73 | 39.05 | -4.1% |
| 8 delays | 110.24 | 106.10 | -3.8% |
| 1 distortion | 38.15 | 38.72 | +1.5% |
| 8 distortions | 123.22 | 118.94 | -3.5% |
| 1 wah | 41.07 | 41.68 | +1.5% |
| 8 wahs | 203.55 | 184.64 | -9.3% |

Both measurements generated the exact same benchmark PCM hash for each case. Differences of a few percent are **not statistically conclusive** on this host, so these readings are evidence of broadly similar or lower cost, **not a promise** of lower CPU on the FM1. Every *additional enabled nonlinear FX instance* still performs useful DSP and has a nonzero cost. No maximum-latency test on physical FM1 hardware is available.

## Resource and safety budget

- V1.94 native app: **401,916 B**. V1.95 native app: **402,144 B**, a **228 B** increase in program flash size, still **179,420 B** below the audited app capacity.
- No extra PCM chorus, delay, reverb, or other audio history buffers are allocated. Only per-slot scalar Wah coefficient caches were added (approximately 96 B of SRAM across eight slots, including structure alignment).
- No allocation, file I/O, logging, trigonometric functions, or floating-point processing was added to the audio interrupt.
- The same trusted OTA loader and protected bootheader were checked byte-for-byte. Both FM1 flash JEDEC IDs remain recognized.
- Serial MIDI, USB audio, patch/project data, independent FX banks, GLIDE, bypass fades, and effect order remain intact.
- `release_ready=false`, `physical_fm1_verified=false`: **not certified for live performance** until measured on physical FM1 with 8-FX stress, high-speed playing, continuous MIDI, toggles/reordering, preset changes, and at least a long endurance session.

## Reproduce

```sh
cd rupture-v195
bash tests/run_tests.sh
cc -std=c11 -O3 -w -DRUPTURE_BUILD \
 -Ivendor/etch-v1.88-source/firmware/fm1-native \
 tests/bench_v193_repeat_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c \
 vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c \
 -lm -o /tmp/rupture_v195_bench
/tmp/rupture_v195_bench
```
