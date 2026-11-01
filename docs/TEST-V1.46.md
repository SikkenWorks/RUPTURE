# RUPTURE V1.46 — Native UI & SAW Knob Test Report

## Changes
- Relocated oscillator naming into native header with a verified gap before OCT. Restored PLAY waveform to its previous reserved band, with three completely black rows before knob geometry. SOUND SHAPE has no duplicate oscillator title in its content panel.
- SAW now uses four musically distinct existing knobs (BRIGHT, EDGE, BODY, LEVEL), while CXS macros are unmodified. SOUND SHAPE EDGE also affects the saw oscillator.
- One 32-bit one-pole per-voice filter state, no new table/voice/buffer, all DSP in bounded integer arithmetic. Original CXS codepath and OTA remain unchanged.
- The four shared preset macro values persist with the prior project format; choosing SAW does not create a separate second bank of macro values.

## Tests
- FM1 native pi32v2 crossbuild: PASS. Application 241580 bytes / 581564 bytes flash allocation.
- Native PNG layout: PASS: `play.png`, `play-saw.png`, `sound-shape.png`, `sound-shape-saw.png`; no painted pixel in rows 80..82 or header/body gap; no text overlap.
- SAW four-knob PCMs: PASS under UBSan; each knob produces different rendered samples; sample values remain bounded even under alternating worst-case input. SOUND SHAPE EDGE is independently audible in the test.
- V1.45 CXS/SAW+SUB waveform and screensaver loops: PASS.
- V1.45 PJT34 codec roundtrip: PASS, legacy CXS defaults and SAW/sub types preserved.
- V1.42 OCTAVE FUZZ stability + fixed reciprocal: PASS.
- V1.43 native OTA staging / boothead guards with V1.46 FWSC: PASS.
- Site tests + firmware embedded byte parity: PASS.
- Full historical host regression suite: NOT RUN to completion for V1.46; targeted existing tests listed above.
- Target hardware DMA and LCD tests: NOT PERFORMED.

## Remaining real hardware validation
The firmware was cross-compiled and host-tested, not physically installed or profiled by the assistant. Screen LCD precision, tactile encoder steps, subjective SAW tone, actual DMA slack and cross-firmware OTA return must be checked with hardware. Back up projects and preserve a rescue route before flashing.
