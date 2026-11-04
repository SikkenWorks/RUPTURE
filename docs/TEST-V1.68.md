# RUPTURE V1.68 — Sequencer Step 1 / REC Playhead Fix

**Status:** Software candidate compiled and tested using the native JieLi pi32v2 toolchain. **Not tested on a physical FM1 in this session.**

## Reproduced defects and changes

1. The legacy native `etch_seq_toggle()` refuses to delete a trigger with a directed HOLD referencing it. This was reproduced with a note on first step (index 0) and HOLDs on steps 2/3. New RUPTURE-only `rupture_seq_entry.h` removes that note and only its directed HOLD dependents; unrelated same-pitch notes, independently authored legacy HOLDs, other pitches and parameter locks survive. No pattern format changes. The normal key-toggle remains intact on all steps. When stopped, SEL also deletes the displayed note on the selected step.
2. The RUPTURE LCD redraw code did not treat sequencer playhead advances as drawable events. LED output already followed the audio step. V1.68 compares the audio playhead with the last actually displayed step and includes pending playhead motion in the existing visual-event LCD refresh policy. No audio-clock or LED scheduling changes. Under repeated DMA misses the existing LCD safety cap is retained.
3. Recording is labeled **REC** in the sequencer header instead of the generic PLAY label.

## Completed tests

- Native pi32v2 cross-build with `--storage --updater --usb-audio-48k`: **PASS**, app **319,856 bytes**, linker limit **581,564 bytes**, headroom **261,708 bytes**.
- `test_v168_seq_entry_playhead.c` against native performance code: **PASS**, toggle/delete on step 1 and every step 1–128, explicit SEL delete, directed HOLD ownership cleanup, legacy unbound HOLD preservation, parameter lock preservation, invalid pattern transactional rejection, 16-step timeline including step 16 → step 1, 17 expected playhead paints, and DMA-pressure cap.
- `test_v168_seq_renderer.c`: **PASS**, actual 240×240 1-bit renderer visibly differentiates playhead 1/2/16, wraparound, REC status, and note removal on first step.
- `test_v167_knob_focus.c`: **PASS**, all eight FX x four knobs and non-FX pages.
- `test_v166_animation.c`: **PASS**, note-gated PLAY/ENV/LFO/SHAPE loops, shaking and closures.
- `test_dam_sprites.py`: **PASS**, all 20 original PLAY PNG frames rendered faithfully with established V1.58 one-pixel shake. Historical test expectation corrected, no artwork or animation code changed.
- `test_v165_png_parity.py`: **PASS**, 60 original ENV/LFO/volcano PNGs byte-identical to native sprite data.
- `test_v166_cxs_weight_punch.c`: **PASS**, WEIGHT/PUNCH sustained changes and amplitude guard.
- `test_v155_cxs_pressure.c`: **PASS**, PRESSURE audible response across full knob range.
- `test_v163_edit_safety.c`: **PASS**, unsaved indicator, undo and project warnings.
- `test_v160_welcome.c`, `test_v164_lcd_budget.c`, `test_v146_ui_layout.py`: **PASS**.
- `test_v143_ota_reclaim.c` with actual `RUPTURE-V1.68.fwsc`: **PASS**, valid native app update, rejected unknown/broken staging, corrupt loader, and bootheader mismatch with no unapproved erase.
- Protected first 0x4000 bytes of flash header and complete OTA-loader component: **byte-identical to V1.67**.
- Source audit against V1.67 ZIP: **161 native source files unchanged**, two changed (`bringup.c`, `rupture_ui.c`), two new headers, **no removals**. Audio, flash, OTA and recovery implementation files unchanged.
- Site/embedded WebInstaller `.fwsc` SHA/length and documents: **PASS**.

## Not completed / physical verification required

- On the actual FM1: enter SEQ, select STEP 1, add a note, tap the same pitch again to erase; repeat with a loaded project containing old HOLD references. Verify SEL deletes only the selected note while stopped. Test 16-, 32-, 64- and 128-step projects.
- Record live at multiple BPMs. Confirm the graphical playhead follows the illuminated white step LEDs, including the 16 → 1 wrap, and no audio crackles under FX/USB load.
- SAVE, power-cycle, LOAD and verify changes persist.
- Physical cross-firmware OTA and official V15 return remain unverified. Use backup and proven recovery hardware. Host OTA verification does not establish a recovery guarantee.
- The full historical regression suite was not completed in this pass; only the targeted cases listed above were run.
