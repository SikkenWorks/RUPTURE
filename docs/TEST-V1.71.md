# RUPTURE V1.71 – Optional 43-Screen Tutorial – Software Test Report

## Build

Native M-VAVE FM1 JieLi pi32v2 application compiled with `--storage --updater --usb-audio-48k`, without modifying the audio callback or the storage/OTA code. Application **340,296 bytes** of 581,564 available, **241,268 bytes headroom**. Firmware `.fwsc`: **610,086 bytes**, identity `FM-1_968`. Native application ELF, package integrity and CRC checks passed.

## Tutorial behavior tested

- First-run flow: original introduction, backup confirmation, deliberately held SAVE for format, persistent format result, and CRT preference remain. Setup complete now leads to **VIEW QUICK TUTORIAL?**. YES enters; NO exits; neither decision auto-formats flash.
- **43 actual pages**: PLAY, SOUND SHAPE, SUB 1/2, menu, FILTER, ENV/LFO, all eight FX, CHANNEL EQ, SEQ, SEQ SLOTS, ARP, SCALE GUIDE, KEYLOCK, NOTE MEMORY, MIDI, project/name management, GLO, SETTINGS, MASTER VOLUME, AUTO LOAD, STORAGE and submenus, SCREENSAVER, SYSTEM INFO, AUDIO DIAGNOSTICS, HELP, ABOUT, QR and WIPE ALL (read-only preview). Runtime-only error/update/loader states are intentionally not represented.
- Per page: route to the real page plus descriptions for each applicable K1–K4 control; supplemental ALGORITHM BLEND instructions on both SUB pages. All strings checked to fit 36-character 1-bit display width. Non-editable pages do not invent knob actions.
- **SEL NEXT**, **SELECT BACK/FORWARD**, **ALGORITHM TEXT/FULL SCREEN**, **HOME EXIT**. Tested selection limits, last page finish, replay exit and SAVE suppression in tutorial.
- Native 240×240 renderer tested with overlay and full page across all 43 lessons. Removing the tour overlay restores exactly the original framebuffer above the footer, verified for each lesson. Screenshots exported via `tests/export_v171_tour.c`.
- GLO adds **QUICK TOUR**, accessible to previously set-up devices. Replay retains prior welcome completion and does not enqueue a new global-flash journal write. Full on-device interaction must still be tested physically.

## Regressions and protection

- Native welcome/format and quick-tour regression, 43-screen renderer, knob focus, ENV/LFO hold loop, sequencer step-1/HOLD deletion, and LCD scheduler: PASS.
- V1.69 fixed golden PCM FX/renderer reference (`tests/check_v169_parity.py`): PASS on all three golden tests. This is sample parity for the test scenarios, not proof of physical FM1 timing.
- Compared V1.71 against V1.70 source ZIP: **164 original native C/H/assembly files byte-identical**; only `bringup.c`, `rupture_ui.c`, and `rupture_welcome.h` changed. New `rupture_tutorial.h` holds read-only lesson metadata. Audio, FX, storage and OTA implementation unchanged.
- 16 KiB protected bootheader and **6,930-byte trusted OTA loader byte-identical** to V1.70; original recovery identity `FM-1_968` retained.
- Native OTA host replay with actual V1.71 firmware: valid app-only update accepted; unknown staged data, modified loader, invalid JLFS CRC and bootheader mismatch all denied without unauthorized erase.
- Site / WebInstaller: embedded firmware raw bytes and SHA-256 match release `.fwsc`; static page integrity tests PASS.

## Physical FM1 test checklist (NOT performed)

1. Cold-boot new storage and an existing initialized FM1: verify welcome YES/NO and that no project data is changed by skipping or completing the tour.
2. Choose YES: check each route, all 43 pages, SEL progression, SELECT back/forward and ALGORITHM hide/show; test HOME early exit.
3. On an already-configured FM1, open **GLO → QUICK TOUR** and verify the first-run backup/format questions do not reappear; confirm project settings and persisted welcome state survive reboot.
4. Play/hold notes, animate knobs and use the 128-step sequencer while replaying the tour, check no DSP/LED timing regressions or parameter edits.
5. Test physical OTA transitions to other trusted firmwares and official V15 recovery with pre-existing backups and a verified rescue procedure. These have NOT been proven by host simulation.

**Status: software-validated candidate; physical FM1 and V15 OTA restore unverified.**
