# RUPTURE V1.42: OCTAVE FUZZ audio stabilization

## Issue

Physical FM1 reportedly produces occasional crackle from OCTAVE FUZZ even after earlier general FX/EQ changes. This is a user-observed problem; no direct capture of physical DAC/audio DMA was available to this build.

## Source-level causes found

- The prior loudness matching updated the gain itself **once every 32 samples**, causing a discontinuous amplitude change that repeats periodically during a sustained note.
- DRIVE, OCTAVE and TONE were read in the audio interrupt and applied without any slew, so abrupt encoder adjustments could generate audible sample discontinuities.
- The absolute-value octave rectifier contained a sharp cusp at zero-crossing, adding unnecessarily broadband high harmonics to an already aggressive fuzz.
- A variable integer division in the periodic gain-tracking update made the hot path's execution time less predictable on the JieLi processor.

## Changes

1. DRIVE / OCTAVE / TONE coefficients are prepared outside the audio callback and slewed per sample. MIX's existing fade is retained.
2. A small C1-smooth (first-derivative continuous) rectifier knee within +/-4096 PCM reduces sharp zero-crossing artifacts while preserving ordinary rectification outside the knee and exact digital silence at zero.
3. Gain targets remain envelope-driven, but gain now ramps **per sample** rather than changing every 32 samples.
4. A ~2 KiB const Q24 reciprocal lookup replaces division by the changing wet envelope in the callback. Target error remains below 400 Q15 units over the entire documented range. No dynamic allocation or additional delay-line buffers.
5. Retained existing fuzz hard-clipping identity, other FX/DSP, all saved project records, MIDI, UI and DMA audio-stop guard. This changes the OCTAVE FUZZ transfer behavior slightly near zero crossings by design, but does not remove octave-up content.

## Directed test outcomes

| Test | V1.41 | V1.42 |
| --- | ---: | ---: |
| 110 Hz / 9500 PCM, rapid extreme DRIVE/OCTAVE/TONE switches, largest adjacent-sample jump | 17,881 | 1,506 |
| RMS of steady extreme OCTAVE FUZZ, identical input | 6,994.5 | 6,987.5 |
| Silent input, OCTAVE FUZZ enabled | 0 output | 0 output |
| After effect OFF and fade | Exact dry | Exact dry |
| 3 million sample host benchmark (`gcc -O2`, reference run) | 140.03 ms | 127.68 ms |
| FM1 crossbuild | Existing V1.41 | V1.42 compiled |

**Interpretation:** These measurements are host tests, not acoustic or physical FM1 tests. The host benchmark reflects the execution environment, not JieLi DMA headroom. The large jump reduction is due primarily to smoothing the previously discontinuous parameter transitions; no claim of absolutely zero analog/alias-related distortion is justified.

## Retained limitations

The FM1's real DAC output, CPU cycles per DMA block, USB and interface interaction were not accessible to the test runner. Continuous multiminute physical monitoring under varying DRIVE/OCTAVE/TONE, multiple FX and full battery/USB scenarios is still needed. Watch AUDIO DIAGNOSTICS `LATE`, `GUARD` and `MIN SLACK`. Do not disable DMA safety for this test.
