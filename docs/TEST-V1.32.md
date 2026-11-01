# RUPTURE V1.32 — Test results

- Native pi32v2 crossbuild: PASS, 215016 application bytes, 366548 bytes remaining.
- Original user-provided Utrecht skyline used; FM1 240x240 pixel animation rendered from native C: PASS.
- Screensaver ENABLED / START AFTER options: UI render check PASS.
- Configurable 15/30/45/60/90/120/180/300 second timer, 30 second default: source and preference tests PASS.
- V1.32 8-byte global journal: old records read with 30 second defaults, timer survives volume writes and new project A/B commits: host flash tests PASS.
- REC LED native control policy: only lights during native_seq_recording; source check PASS.
- Website header R glyph: removed from website template.
- Protected firmware first 0x4000 bytes: identical to V1.31, PASS.
- Trusted OTA loader: identical to V1.31, PASS.
- Firmware .fwsc CRC: PASS.
- WebInstaller firmware byte identity: PASS.
- Firmware bytes: 610086; SHA-256: e2930572c3187ff0cafe874ce09c79d70351442ba32a9b89c65f861f08923800.
- Historical regression suite: all earlier stages through V1.27 passed before the source-contract update; V1.28-V1.32 targeted storage, sequence, UI and site checks passed independently. Whole suite was not re-run to completion in a single run.
- Physical FM1 input / live REC light / USB OTA / LCD framerate: NOT VERIFIED by this environment. Test on an actual FM1 before broader release.
