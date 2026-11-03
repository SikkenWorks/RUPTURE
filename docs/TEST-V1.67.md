# RUPTURE V1.67 — Consistent Knob Focus / Release Verification

**Status:** Native software build and selected host regressions PASS. **Not tested on a physical FM1.**

## Fix and scope

- Root cause: `ru_adjust()` enabled focus only for PLAY, SHAPE, SUB, FILTER, ENV, LFO and SETTINGS. All FX controls route through temporary per-effect pages; the visual FX page therefore never received focus notification.
- Fix: A shared focus decision is applied after parameter dispatch/FX forwarding. FX tracks the visible page, uses the existing 950 ms timeout, and triggers immediate refresh. Focus is reset on navigation, effect selection or EQ tab changes.
- All standard rotary pages use short side-of-knob bars. The SEQ renderer now calls the same marker helper. SUB's fifth ALGORITHM/BLEND rotary also uses it. EQ row labels receive a separate left-gutter mark rather than fake knob circles; SETTINGS volume values get a gutter marker.
- No audible or persistent behaviour was intentionally changed. No flash write, storage format, animation scheduler or DSP path was modified.

## Completed checks

- Native JieLi pi32v2 cross-build (`--storage --updater --usb-audio-48k`) PASS: **319,496 bytes of application** against 581,564-byte limit, **262,068 bytes free**.
- Native packed 240×240 renderer regression `test_v167_knob_focus.c`: PASS, all 8 FX choices × all four K-controls, 21 other rotary UI pages × K1–K4, both fifth SUB BLEND knobs, and both EQ value layouts × K1–K4.
- V1.66 PLAY/ENV/LFO/SOUND SHAPE loop, shake and release animation regression: PASS; 135 waterloop transitions observed in test.
- V1.66 CXS WEIGHT/PUNCH sustained-tone regression: PASS.
- V1.55 PRESSURE audio regression with UBSan: PASS.
- V1.63 project dirty/undo state regression: PASS.
- V1.60 first-run wizard regression: PASS.
- V1.64 adaptive LCD budget test: PASS.
- V1.64 SYSTEM INFO status regression: PASS.
- V1.65 original sprite parity: PASS, 60/60 original PNGs unchanged.
- Native OTA replay of V1.67 `.fwsc` (`test_v143_ota_reclaim.c`): PASS valid install, fail-closed invalid loader/staging/bootheader, no unintended erase in rejection cases.
- The protected first 16 KiB bootheader and the entire OTA-loader component are byte-identical to V1.66.
- Source audit: **161** native C/header/assembly files byte-identical to V1.66; changed only `bringup.c` and `rupture_ui.c`. Key audio, OTA, flash storage and project serialization sources were compared byte-for-byte unchanged.
- Site, embedded installer FWSC checksum and metadata checks: PASS (`build_site.py`, `test_site.py`, `test_website_release.py`).

## Still needs physical FM1 testing

- Turn K1–K4 in all eight FX views and confirm visible ~950 ms focus; change effects rapidly and ensure no stale marker carries across.
- Confirm SEQUENCER and EQ markers do not collide with graphics, and SUB BLEND highlights when ALGORITHM is turned.
- Verify normal sound, animated panels under load and project SAVE/LOAD on the FM1.
- Verify cross-firmware OTA and official stock V15 recovery physically using a proven recovery setup; package parity and host OTA simulation alone do **not** prove hardware recovery.

Full historical test suite not rerun for this small UI-only change.
