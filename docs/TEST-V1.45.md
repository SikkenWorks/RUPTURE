# RUPTURE V1.45 – Native Release Verification

Date: 2026-11-01

## Implemented

- Main oscillator: CXS or SAWTOOTH selected by ALGORITHM on PLAY and SOUND SHAPE. ALGORITHM on SUB 1 and SUB 2 remains independent BLEND control.
- SUB 1 and 2: OFF/CXS/CLASSIC/SQUARE/SAW; existing octave, detune, drift, blend behavior retained.
- Band-limited, offline-generated Q15 SQUARE/SAW mipmaps. No wavetable synthesis, heap allocation or floating-point processing in audio callback. The new waveforms are *additional modes*, not additional simultaneous synth voices. CXS keeps its prior signal path when selected; SAW bypasses steady-state CXS event-generation work. 64-sample click-reducing crossfade on live main oscillator changes.
- Saved main selection in previously unused PJT34 DSP record bit 4; both SUB type codes extend existing tags. No record growth or flash sector changes. Legacy projects default to CXS.
- Screensaver reuses original 64 animation frames: forward, pause at frame 63 for 2200 ms, reverse to 0, continuous repeat. Only main-loop UI indices are changed; audio path remains independent.

## Test results

- FM1 native pi32v2 build: PASS. App size 241180 bytes out of 581564 bytes, leaving 340384 bytes of allocated application flash.
- V1.45 new oscillator/legacy-default/SUB signal/wavetable/animation-loop test: PASS, with UBSan.
- V1.45 PJT project encode/decode: PASS for new main SAW bit, two new SUB types, legacy defaults and reserved bit validation.
- V1.38 all-filter stress (2.5M samples): PASS, identical legacy processing.
- V1.42 fuzz stability regression: PASS.
- V1.43 OTA staging/recovery guards: PASS using new V1.45 FWSC.
- V1.44 official stock V15 native CHECK→commit handoff regression: PASS using archived official V15 and previous RUPTURE boot reference; all corrupt/unknown cases rejected.
- WebInstaller embedded firmware / firmware SHA-256 / website source parity: PASS.
- Historical `tests/run_tests.sh`: initial tests through V1.17 passed; full suite reached execution time limit before completion, so the entire suite is **not** claimed passed.

## Limitations

This is a **cross-compiled, host-tested firmware candidate**, NOT a physical-FM1 DMA or audio quality measurement. More modes necessarily use more flash (roughly 8 KiB vs V1.44) and a few branch cycles; claims of literally zero CPU overhead would be inaccurate. When SAW is selected, the costly CXS carrier is skipped after the short mode fade. New square/saw subs use a compact integer lookup rather than the more expensive original CXS sub engine. Actual worst-case DMA slack and musical quality on the target FM1 need live confirmation. Back up user projects and retain a verified rescue path before flashing. The historical loader and boothead have not been changed.
