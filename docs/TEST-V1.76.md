# RUPTURE V1.76 — Quantization, LED Flicker and First Downbeat

## Scope

This is a source-level and host-tested follow-up to RUPTURE V1.75 for M-VAVE FM1. No physical FM1 test or real flashing was carried out here.

## Changes and root causes

1. **Nearest-grid live REC:** Previously the UI inserted notes at the current step without compensating for LCD/UI tick latency or user key-contact timing. The new UI-only quantizer uses the native audio playhead, step duration, swing and the TIMER4 timestamp captured at physical first contact. Hits are rounded to the nearest step across the pattern boundary. Note sustain tracks rounded note origins and avoids incrementing future-step notes prematurely.
2. **First key after countdown:** In the count-in transition, the ISR could already be playing step 0 while the UI still had `native_seq_run == 0`. The original call to `etch_seq_actual_step(native_seq_run,...)` then fell back to the manual edit cursor, putting the first hit on the wrong step. The fixed handler uses the effective transport state (`native_seq_run || ru_countin.force_seq`). A bounded 50ms preroll key latch covers physical contacts just before the boundary; a first-step guard prevents invalid back-quantization into the previous loop. The ISR temporary force flag is cleared when the UI has acknowledged the start, allowing subsequent STOP.
3. **Half-bright LEDs:** The existing LED multiplex tick is about 909 Hz per complete scan. V1.75 held the intermediate brightness for one frame out of eight (roughly 114 Hz) or one out of twelve in DIM LO (76 Hz). RUPTURE V1.76 changes only RUPTURE compile-time configuration to one in two (about 455 Hz) and one in three (303 Hz), respectively. Original Felucca `hal/fm1_input.h` remains unmodified. This is a higher-duty steady brightness, not additional interrupts or audio samples. Whether every user perceives the LEDs as flicker-free must be checked physically.
4. **Preserved features:** two-bar count-in, independent PCM metronome and BPM/ALGORITHM mapping, sustain recording, SEQ+PRESETS rotation, entire step clear, ENV animation, project codec, USB/OTA and trusted recovery components.

## Build and release integrity

| Item | Verified result |
|---|---|
| JieLi pi32v2 native crossbuild | PASS |
| Second crossbuild | PASS, byte-identical application |
| App size | 345,868 bytes / 581,564; 235,696 bytes free |
| Firmware package | 610,086 bytes, parsed and verified CRC/structure |
| Firmware identity | `FM-1_968` unchanged |
| Application SHA-256 | `8033eef3b34e147ad7a4e857f6ee8facefcde2c0bc437dbd401140780bd290f7` |
| FWSC SHA-256 | `8cb7b8bb7369a84398a0e2bcd265c18d4c43a4ba858a57395388f2fe31a80536` |
| Original first 16 KiB from V1.71 | Byte-identical |
| Original trusted OTA loader from V1.71 | Byte-identical |
| Native OTA handler and storage codec versus V1.75 | Byte-identical |
| Felucca native HAL | 19 upstream file hashes audited and retained |
| WebInstaller and GitHub Pages embedded FWSC | Exact binary equality |

## Host tests executed

- `test_v176_quant_led.c`: nearest step with swing, loop wrapping, timestamp correction, delayed UI, pending/future and prior-step sustain: PASS.
- `test_v176_countin_bridge.c`: key latch only near bar boundary, first-step guard, forced-transport cleanup: PASS.
- `test_v176_source_contract.py`: actual native entry-point source checks and unchanged native OTA/storage/HAL: PASS.
- `test_v175_countin.c`: eight exact count-in clicks, tempo changes, abort, 30/60/120/240 BPM: PASS.
- `test_v175_transport_contract.py`: REC/PLAY and independent click behavior: PASS (firmware-info version assertion updated to V1.76).
- `test_v175_record_ui.c`: 240x240 countdown states: PASS.
- `test_v174_seq_shift_sustain.c`: sustained notes and lossless pattern shifts: PASS.
- `test_v172_metro_and_step.c`: independent click and full step clear: PASS.
- `test_v123_help_cards.c`: 13 native help cards: PASS.
- `test_v171_tutorial.c`: 43 Quick Tour screens and read-only tutorial: PASS.
- `test_v173_mapping_docs.py`, `test_website_release.py`: PASS.

The complete historical stress/regression suite was **not** rerun. Real FM1 behavior, perceived LED flicker, physical key response, CPU overhead and firmware recovery remain unverified. Back up projects before installing and keep a tested external recovery path available.
