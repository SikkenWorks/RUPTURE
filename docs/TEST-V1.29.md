# RUPTURE V1.29 — Firmware, SETTINGS and GLO tests

## Firmware

- **Native FM1 cross-compiler build: PASS** (JieLi pi32v2, `--storage --updater --usb-audio-48k`). 208,792-byte application with 372,772 bytes application headroom.
- **Firmware verification: PASS** (internal package CRC and FM-1_968 identity). 610,086-byte `.fwsc`.
- **Protected first 16 KiB and OTA loader: PASS**, byte-identical to retained reference.
- **Website / installer embedded firmware byte parity, SHA-256 and length: PASS**.
- **Native 240×240 screen exports: PASS** (52 images); GLO shows complete `AUDIO DIAGNOSTICS`, SETTINGS shows live MASTER percentage and direct AUTO SAVE checkbox.
- **Native UI and input-routing source checks: PASS**. AUTO SAVE uses SEL directly, not a modal; MASTER K1 adjusts on the selected row and SEL retains the detailed page. OTHER/SCREENSAVER remains a direct SEL toggle.
- **Previous V1.25, V1.26 and V1.27 control/source compatibility tests: PASS**.
- **V1.28 sequence-copy/delete test: PASS**. V1.28 visual state checks: PASS.
- **Full historical regression test: INCOMPLETE (time limit)**. It ran successfully from the earliest CXS tests through the V1.24 UI and audio tests before its execution limit, but was not certified to completion. Remaining targeted controls passed independently.
- **FM1 physical test for V1.29: NOT PERFORMED HERE**. Verify on an actual unit, especially SEL toggling, K1 global master volume before entering its subpage and persistent settings after reboot.

## Compatibility / safety

This version changes menu rendering and the handling of selected physical controls only. No audio DSP source, project/sequence storage layout, NOR sector map, OTA loader or first 16 KiB bootheader is changed. A backup and proven recovery path remain necessary for physical firmware updates.
