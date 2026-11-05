# RUPTURE V1.84 — Hardware Integration Test Record

## Scope

Source baseline: **V1.83**. This release changes only the UI control loop (`bringup.c`), a small pure function LED policy header (`rupture_hw_leds.h`), the visible version and supporting documentation. It does not change the native audio-processing source, the three reverb algorithms, key/encoder hardware scan ISR, persistent storage, the Welcome completion journal, or native OTA code.

## Hardware-facing changes

- Page-aware physical function button LEDs: active page brightly indicated; inactive function buttons use existing high-frequency dim pulses, without a new slow pulse source.
- Button press takes priority, and the sequencer play/record/count-in states have explicit status ownership. Note, keylock, octave, GRID and MIDI output behavior is preserved.
- K1–K4 fast-turn acceleration history resets on page transitions and between the FX panel categories. A newly selected page starts with single-detent precision; normal fast-turn acceleration still applies to continuous controls within a page.

## Verification performed in this container

| Check | Result |
|---|---|
| JieLi `pi32v2` crosscompile, storage, OTA and 48 kHz USB-audio build flags | PASS |
| Native app/linker validation and available app region | PASS |
| V1.84 function LED mapping, physical press priority, protected screens and two-bar countdown | PASS |
| V1.84 page-change encoder reset + discrete step handling | PASS |
| Historical V1.62 knob focus and encoder host regression | PASS |
| Historical V1.75 countdown screen render test | PASS |
| Historical V1.81 burst-LCD budget test | PASS |
| V1.82 vs V1.83 audio note/FX host comparison (unchanged V1.83 code in this release) | PASS |
| Byte-for-byte source audit: `audio_core.c`, `native_fx.c`, `wave_store.c`, `ota.c`, `hal/fm1_input.h`, `rupture_welcome.h` | IDENTICAL to V1.83 |
| Protected 16 KiB header and trusted OTA loader in new `.fwsc` vs V1.83 | IDENTICAL |
| Installer embedded `.fwsc` and release SHA256 | VERIFIED |
| Website build and documentation embedded data parity | VERIFIED |

## Not performed / limitations

- **No test on a physical M-VAVE FM1:** actual perceived brightness, electrical scan timing under load, encoder ergonomics and full button behavior must be checked on the device.
- This is **not** proof of zero DMA LATE/GUARD under rapid notes or maximum effects. Compare *counter increments* and minimum slack on physical hardware.
- The entire historical test suite has **not** been rerun; targeted regressions only.
- OTA integrity is checked statically and at the package level; the cross-firmware update/recovery process was **not** physically exercised. Keep a verified recovery option and back up projects before installing.

## Target acceptance checklist on hardware

1. At PLAY, HOME is bright, other function LEDs retain a dim glow. Press ENV, FX, EDIT, ARP, SEQ and GLO and verify the correct bright button follows the page.
2. Start playback: PLAY LED bright; arm REC: REC LED bright; trigger a two-bar count-in: SEQ LED pulses once each beat without affecting held notes or metronome timing.
3. Hold or tap inactive buttons and confirm that the pressed button is immediately bright. Note and octave LED behavior must remain unchanged.
4. Turn K1 rapidly on PLAY, switch to SUB or ENV, then turn K1 once: first detent on the new page must change the parameter by one normal increment. Repeat when switching FX subpages.
5. Recheck fast-note playback with both SUBs, DRIFT and ROOM/SPRING/HALL at severe settings, monitor DSP, DMA LATE, GUARD and MIN SLACK before/after.
6. Verify SAVE/LOAD and a routine RUPTURE update with your proven recovery route available.
