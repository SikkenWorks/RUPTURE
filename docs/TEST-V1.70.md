# RUPTURE V1.70: ENV / LFO hold animation verification

## Build status

**Software-validated FM1 candidate. Hardware animation timing, actual DSP headroom and cross-firmware/stock V15 flash recovery remain untested.** Do not interpret host OTA simulation as a physical flash or rescue guarantee.

Native build: JieLi pi32v2 toolchain, `--storage --updater --usb-audio-48k`, identity `FM-1_968`. Native app: **334,492 bytes**, 247,072 bytes application headroom, entry point `0x02000120`; ELF and app audits passed. `.fwsc` container: 610,086 bytes; parser and CRC/identity validation passed.

## Source art and animation behavior

- 8 new pure 1-bit **224×30** ENV smoke-settle sprites and 8 new LFO falling-debris hold-loop sprites, plus reproducible Python source generator and PNG/GIF previews.
- Native 1-bit packed sprite arrays match all 16 PNGs byte-for-byte. Their outer scene pixels are unchanged relative to the existing open wall/sinkhole, and the fully settled ENV last frame is byte-identical to the original open ENV frame 13.
- All previous 20-frame ENV, 20-frame LFO and 20-frame volcano PNGs are byte-identical to V1.69 (60/60); PLAY's original sprites are unchanged as well. LFO/ENV use the old opening/closing graphics before/after the new hold frames.
- **ENV:** the dust-settle sequence plays once, then the framebuffer is stable for at least 120 consecutive 25-ms service ticks; release starts the original `13..19` closing progression. Very short presses also safely close.
- **LFO:** the note-held sinkhole cycles all 8 falling-debris frames during a 48-tick test across multiple repetitions, then plays the original reverse close after note release. Subtle shake remains.
- A native 240×240 framebuffer test confirms transitions and exact final ENV source pixels. Five actual native renderer previews were exported to `ui/screens/`.
- The time-based native LCD budget remains V1.69's 25-ms animation target when DMA is healthy, without per-particle draw calls. Historical `test_v166_animation` was updated to the already-established V1.69 `100ms` genuine-DMA-miss guard rather than its obsolete `180ms` assertion.
- The new sprites are read-only firmware data. No modifications to the audio ISR, PCM mixer, DSP, MIDI, sequencer, project format or storage controls.

## Regression and safety tests

- `tests/test_v170_env_lfo_hold.c`: PASS; actual native sprite/renderer, long held notes, early note release, endless loop and closing.
- `tests/test_v170_hold_png_parity.py`: PASS; all 16 PNGs identical to ROM, and original panel edges unchanged.
- `tests/test_v156_panel_anim.c`, `test_v157_volcano_anim.c`, `test_v158_panel_shake.c`, `test_v166_animation.c`: PASS; earlier interactions still work.
- `tests/check_v169_parity.py`: PASS; the **800,000-stereo-frame** FX golden checksum set, 12 additional FX reference scenarios and 42 previous non-versioned 240×240 screen fixtures remain byte-identical. This validates output equivalence for the tested audio scenarios, not live performance on the board.
- `test_v168_seq_entry_playhead.c`, `test_v168_seq_renderer.c`, `test_v167_knob_focus.c`, `test_v163_edit_safety.c`, `test_v160_welcome.c`, `test_v166_cxs_weight_punch.c`, `test_v155_cxs_pressure.c`, `test_v169_lcd_and_knob.c`: PASS.
- Original image parity tests: PASS (all 60 20-frame ENV/LFO/volcano images).
- **20 critical audio, OTA and storage source files are byte-identical to V1.69.** The 16-KiB protected bootheader and complete trusted OTA loader inside the new `.fwsc` are also byte-identical to V1.69.
- Native OTA replay with the real **V1.70 `.fwsc`**: PASS for valid app-only update; refusal of unknown staging data, corrupted loader, corrupt JLFS header and mismatched bootheader without unauthorized flash erase.
- Full WebInstaller/site checks: firmware bytes and SHA-256 must exactly match the single active `.fwsc` file. All release files are regenerated together.

## Physical FM1 tests still required

1. Hold a note on ENV and watch the dust settle; confirm the last frame remains still and Note OFF closes correctly.
2. Hold a note on LFO for at least 30 seconds; check continuous dirt fall and slight shake, no jerking or stops; confirm Note OFF restores the road.
3. During both animations, turn K1–K4, play sequences and stress FX; check audio DMA LATE/GUARD counts and actual LCD fluidity.
4. Confirm real SAVE/LOAD and reboot persistence with existing projects.
5. Test actual cross-firmware OTA and official V15 restoration with backups and known-working recovery equipment.

**No physical FM1 was connected in this session.**
