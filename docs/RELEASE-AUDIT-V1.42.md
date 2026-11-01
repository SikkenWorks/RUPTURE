# RUPTURE V1.42 — OCTAVE FUZZ stability release audit

The user's physical FM1 reportedly still has occasional audible OCTAVE FUZZ crackle. This release addresses a reproduced digital discontinuity and related realtime processing defects. It is **not** represented as independently confirmed crackle-free on the physical FM1.

## Source audit

- V1.41 loudness makeup changed gain only every 32 samples. V1.42 computes the target at the existing control rate but applies gain smoothly sample by sample.
- V1.41 applied DRIVE/OCTAVE/TONE directly from asynchronous UI control variables. V1.42 prepares Q15 coefficients in the UI and interpolates them in the audio callback. Existing MIX slew remains.
- Full-wave rectification now uses C1-continuous shaping near +/-4096 PCM at crossings, keeping hard-fuzz clipping and preserving the output at silence and at magnitudes beyond that band.
- Replaced the variable divisor in the loudness tracker with an interpolation-based constant-table reciprocal, held in flash (.rodata), without heap allocation. Maximum denominator target error across the supported range: 367/32767.
- Runtime FX struct gained 5 signed 32-bit control fields. There are no changes to stored PJT34 records, project layout or the trusted OTA image.

## Test evidence

- A 110 Hz test at 9500 PCM with extreme rapid knob toggles produced a largest adjacent-sample jump of **17,881** in V1.41 versus **1,506** in V1.42, a 91.6% reduction under those conditions.
- Steady RMS amplitude was 6,994.5 in V1.41 and 6,987.5 in V1.42 (~0.1% difference). The change deliberately smooths zero-crossing behavior and does not promise sample-identical fuzz timbre.
- Exact zero output for digital silence with FUZZ on; exact dry PCM after effect OFF and fade complete.
- Host benchmark of 3,000,000 FUZZ samples (`gcc -O2`): 140.03 ms before and 127.68 ms after in one run. This is not a cycle-accurate FM1 performance measurement.
- Targeted FX-bypass, compressor, max-FX stress, dual-sub+FX, 12 scenarios and EQ2 regression tests passed.
- Full `tests/run_tests.sh` completed with exit 0, including newly added V1.42 tests.
- UBSan test of sustained FUZZ and transitions passed without undefined-arithmetic warnings.
- Two native JieLi pi32v2 crossbuilds produced the same application binary. Application code is 232,144 bytes within the 581,564-byte application FLASH limit. The difference is NOT CPU or DMA headroom.
- Identical V1.42 FWSC bytes in standalone WebInstaller and index.html; CRC and SHA-256 verified.
- First 0x4000 flash bytes and OTA loader body are byte-identical to V1.41. No change to the audio deadline guard, USB identity, flash layout or sequencer data.

## Physical test still required

Capture real FM1 playback through headphones/audio interface with OCTAVE FUZZ enabled, including DRIVE and OCTAVE 100%, all TONE positions, sustained low/high notes, quick knob sweeps, wet/dry toggling and FX combinations. Inspect `GLO → AUDIO DIAGNOSTICS`: `LATE=0`, `GUARD=0` and positive `MIN SLACK` should remain stable. If crackle remains, record a short audio clip and these numbers with the same patch and report how they differ from V1.41. Do not disable audio DMA protection.
