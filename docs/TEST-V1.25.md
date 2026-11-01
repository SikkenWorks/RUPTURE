# RUPTURE V1.25 test results

- JieLi pi32v2 native cross-build: PASS (not a physical FM1 boot test).
- Full host regression suite including dual SUB + FX stress and PJT33 storage: PASS.
- Native 12-slot NOR emulator: initial blank-bank SAVE, 12 projects, CRC, torn-write recovery, sector/page-bound programs, and real progress phase counters: PASS.
- USB runtime string and loader identity source audit: PASS. Actual OS pairing must be checked after reconnect.
- Native LCD 1-bit project SAVE/LOAD overlays exported from the same firmware renderer: PASS.
- CHANNEL EQ signed +/- host PCM test: treble can rise or fall without material loss of lows: PASS.
- Runtime hardware read/write and uninterrupted real-time display: NOT VERIFIED. No physical FM1 was available to validate SAVE retries or flash progress timing.
- RUPTURE native LCD progress during separate OTA loader WRITE: NOT IMPLEMENTED. Preserved trusted OTA loader cannot run RUPTURE app UI. Browser WebInstaller progress unchanged.
