# RUPTURE V1.73 — native build and verification

## Change scope

- **PRESETS** remains the BPM encoder, including while live sequencer REC is active. The existing BPM change routine and project tempo field are retained.
- **ALGORITHM** controls the independent metronome click level **0–100 only while the sequencer is live recording with the transport running**. When REC stops, its original functions (including sequencer note duration/HOLD editing) resume. REC events are consumed before other ALGORITHM routing.
- The metronome audio mixing code, ENV eight-frame settling and shake animation, and sequencer full-step deletion implementation are unchanged from V1.72.
- The real on-device sequencer footer, 43-screen Quick Tour's sequencer lesson, dedicated HELP card (now 11 topics), README, cheat sheet and embedded GitHub Pages documentation show the correct controls.
- User projects, native OTA code, OTA loader, protective flash bootheader, firmware identity `FM-1_968`, storage layout, and recovery path are not modified.

## Executed checks

| Check | Result |
|---|---|
| JieLi pi32v2 native build, storage + OTA + USB audio 48 kHz | PASS |
| Second full native build byte-for-byte identical | PASS |
| Application entry point and flash size audit | PASS, 0x02000120; 342,064 / 581,564 bytes; 239,500 bytes free |
| `test_v173_record_controls.c` | PASS: REC-only ALGORITHM volume, 0/100 clamp, original non-REC behavior, PRESETS BPM calculations |
| `test_v173_mapping_docs.py` | PASS: control routing and all current on-device/web documentation texts |
| `test_v172_metro_and_step.c` | PASS: original independent metronome trigger and full-step deletion regressions |
| `test_v171_tutorial.c` | PASS: every one of 43 on-device tour pages and read-only navigation |
| `test_v123_help_cards.c` | PASS: all 11 scrollable HELP cards |
| `test_v170_env_lfo_hold.c` | PASS: settled ENV dust, continued wall shake, Note OFF close and unchanged LFO loop |
| `test_v168_seq_entry_playhead.c` | PASS: step 1, linked HOLDs, 128 steps, REC playhead |
| `test_v156_panel_anim.c`, `test_v158_panel_shake.c`, `test_v166_animation.c`, `test_v169_lcd_and_knob.c` | PASS |
| `test_v170_hold_png_parity.py` | PASS: all eight ENV and preserved LFO source PNGs |
| `tools/test_site.py` and `tests/test_website_release.py` | PASS: website/installer/readme/cheat/firmware parity |
| `tools/check_release_clean.py` | PASS |
| FWSC package CRC and native application layout | PASS |
| V1.72 vs. V1.73 protected 16,384-byte bootheader | Byte-identical |
| V1.72 vs. V1.73 trusted `ota.bin` loader | Byte-identical |
| V1.72 vs. V1.73 `audio_core.c`, `ota.c`, step-clear implementation and ENV scheduler | Byte-identical |
| Browser OTA protocol JS | Same as V1.72 except two user-facing version labels |
| Physical FM1 boot, controls, OTA transition and audio DMA tests | NOT performed |

## Release hashes

- Firmware `.fwsc`: SHA-256 `b2d505439ab1bf2d04db15c97a45319ab0509182bc531a6664a0871938d3ff9e`; size 610,086 bytes.
- Application `app.bin`: SHA-256 `297087a2402d047ed82576c4c61b6ecfe6266e4e54c2d1e2d3ab5522a0493503`; size 342,064 bytes.

## Hardware limitations and safety

The release has been **built and tested on host and cross-toolchain only**. Actual metronome audibility, PRESETS/ALGORITHM encoder behavior on the physical FM1, maximum-load audio DMA, flash persistence, loader handoff and successful physical update or recovery have not been verified in this session. Make a project backup, keep an independent working FM1 recovery path available and do not interpret a passing simulator or native package test as guaranteed-safe hardware flashing.
