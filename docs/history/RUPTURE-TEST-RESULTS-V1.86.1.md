# RUPTURE V1.86.1 — K3 BEND Curve Presets: build/test report

## Change scope

- Based on **V1.86 Connection Fix** (not the pre-fix installer).
- BEND screen K3 selects `LINEAR`, `SOFT`, `HARD`, `S-CURVE`, `EXP`.
- Curves control the K1 bend response and the return after K1 stops; all have the same exact K2-selected maximum (¼, ½, 1, 1½ or 2 semitones).
- The original full-resolution **eight-frame packed 1-bit mountain sprite sheet is byte-identical**. Only its selected frame follows the curved strength.
- The display uses three compact static-pixel knobs, a curve graph, the actual curved live pitch and bend meter.
- No on-flash storage format change. BEND profile is momentary session/performance state, defaulting to LINEAR for unchanged V1.86 response.
- No new sample-loop or audio-ISR work; the bend multiplier continues to be calculated in the 25ms control loop and applied to the oscillator **before** the existing FX engine.

## Native build and tests

- JieLi PI32V2 native app compiled with `--storage --updater --usb-audio-48k --multi-project`.
- App binary: **359,128 bytes**, application headroom **222,436 bytes**.
- Two independent FM1 compilations: **byte-identical SHA-256** `2b19d3b18bc1c9fd07e4e6a5c4d5586b13e629e2208b9377a372a4dac98f89ee`.
- Host test: all five curves monotonic and bounded, exact 0%/100% endpoints; LINEAR exact legacy parity; return to original pitch.
- Native DSP host test: all five modes produce distinct sustained **CXS and SAW** PCM at the same K1 position; no note retrigger; return at neutral.
- Prior V1.86 CXS/SAW neutral/live-BEND PCM test passed.
- Prior V1.85.1 SEQ audio test passed: LENGTH 16→4→16, preserving all other notes.
- V1.85 realtime MIDI budget and panel-parity tests passed, along with V1.84 LED and PC6 tests.
- 24 non-BEND UI states compared directly to V1.86 native renderer: **pixel-identical**.

## OTA / WebInstaller / new FM1 flash-chip audit

- Firmware product identity: **`FM-1_968`**; `.fwsc` verified with the existing UFW/FWSC integrity parser.
- The **trusted OTA loader** is byte-identical to V1.86 and V1.85.2 (6,930 bytes, SHA-256 `d71d1b7c12adede951605e758a3580dacccedc427d49e1fceaf51decccc68a4b`).
- The **protected boot/header area** (16,384 bytes) is byte-identical (SHA-256 `a65e0e1a2fad1db943c00b529b45553c524257146145db61b69628cefcf2b2c9`).
- Existing OTA protocol, SPI NOR driver, JEDEC flash policy and `audio_core.c` are unchanged.
- **V1.86 Connection Fix retained** in both index and WebInstaller: `FM-1_968` identity is read up to NUL; 27-byte response JEDEC fields are parsed separately; both 0x856014 and 0xB36014 host simulations pass; unknown Windows MIDI port names use read-only identity probing.
- FWSC, WebInstaller embedded binary and index embedded binary match **byte-for-byte**.
- FWSC SHA-256 `409375091e47b35718bc2f5e4cd78c60b67c500467decda67b113e5cf17b76f1` (610,086 bytes).

## Limits / installation safety

**No physical FM1 flash, MIDI link, speaker or sustained DMA stress test was possible here.** A successful native build, host tests and matching installer prove software-level consistency but do not guarantee hardware behavior on all FM1 batches. Back up the user's projects, use a known working recovery route, start at low volume due to the previously reported power-on speaker pop, and test USB-C connect/disconnect and a cold restart before publishing broadly.
