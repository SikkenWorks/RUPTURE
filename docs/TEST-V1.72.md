# RUPTURE V1.72: build and regression report

## Scope and results

- V1.71 full project is the unmodified base. V1.72 changes ENV settling/shake, the RECORD-only click overlay and RUPTURE sequencer full-step clearing. No existing features or project serialization were removed.
- FM1 JieLi pi32v2 cross-build: **PASS** with persistent storage, native OTA and UAC 44.1/48k flags. Actual physical FM1 verification is **not performed**.
- Application size: **341,784 / 581,564 bytes**; **239,780 bytes headroom**. Boot entry 0x02000120 and app audit pass.
- `tests/test_v170_hold_png_parity.py`: **PASS**. Eight new ENV and eight old LFO ROM bitmaps match their exact 224×30, 1-bit PNG bytes; original ENV opening/closing and other panels unchanged.
- `tests/test_v170_env_lfo_hold.c`: **PASS**. ENV holds last settled frame and continues subtle 1px shake, does not loop the smoke; Note OFF and early Note OFF complete the original closing animation. LFO loops unchanged.
- `tests/test_v172_metro_and_step.c`: **PASS**. Complete step deletion, directed HOLD cleanup, preservation of other notes/HOLDs, corruption-atomicity, 8 sample click events over 32 sixteenth-note steps, and silence when REC OFF or volume zero.
- Metronome is a lightweight original 256-sample PCM click, triggered in the actual sample-rate sequencer tick and added **after** the monophonic synth, effects and idle gate. It does not use voice allocation. It is off during ordinary playback. PRESETS adjusts 0..100 only while recording; the global BPM and preset-selection handlers are explicitly bypassed in that state. Volume is a nonpersistent session setting.
- Older sequencer/animation UI regressions (`test_v168_seq_entry_playhead.c`, `test_v171_tutorial.c`, `test_v169_lcd_and_knob.c`, `test_v156_panel_anim.c`, `test_v158_panel_shake.c`, `test_v166_animation.c`): **PASS**.
- `tools/test_site.py` and `tests/test_website_release.py`: **PASS**. Website, WebInstaller and .fwsc package SHA-256 match.
- Firmware container: **PASS**. V1.71 and V1.72 `flash.bin` share identical first 16,384 bytes; identical `ota.bin` body; identical chip/product ID `FM-1_968`; original native `ota.c` and USB loader unchanged. Two textual V1.71→V1.72 replacements in OTA-browser feedback strings do not alter the transport logic.

## Limitations

The actual physical M-VAVE FM1 has not been tested with this exact build. User recording behavior, on-device metronome audibility and DMA slack, sustained note/FX interaction, storage persistence, OTA loader handoff and cross-firmware restoration must still be confirmed on hardware. Do not interpret host-side tests as confirmed hardware-safe installation. Back up projects and have a working ROM rescue route before flashing.
