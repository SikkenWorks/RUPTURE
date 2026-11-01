# RUPTURE V1.22 test results

**Firmware:** RUPTURE-V1.22.fwsc, 610086 bytes, SHA-256 `6a8d27f2c5a22fd1ccfd9890d993304b17353a9fd3d4cb4460e42df30c6c8fe8`.
**App SHA-256:** `b45e39c7f64dba29dba50df5319fbf7ee4463a018a43dc8016fc7828f876f531`.

- Native JieLi pi32v2 build with storage + updater + USB-audio 48k flags: PASS.
- Existing regression suite: PASS after updating obsolete HIGH CUT and removed-effects expectations.
- V1.22 independent control tests: PASS (Tilt K1–K4; Sandpaper SOFT/WARMTH; Fuzz loudness; parameter sweeps; tagged PJT33 records).
- V1.22 fifth SUB BLEND UI and additive mixing test: PASS.
- Fuzz loudness in 140Hz dual-sine host test: dry RMS 5372 PCM, hot fuzz RMS 5836 PCM. Full drive/octet content unchanged before wet-level normalization.
- Sandpaper rapid SMOOTH, SOFT, WARMTH sweeps: max consecutive sample change 1325 PCM, full-chain peak <=28000 PCM.
- Old project storage/12-slot flash and torn writes: PASS.
- OTA protected bootheader and loader: verified against V1.20 after packaging.
- Physical FM1 subjective clicks, DMA-deadline measurements, and hardware save/downgrade: **NOT TESTED**.

**Compatibility warning:** V1.22 writes PJT33's former Transient fields as SUB BLEND values and marks bit4 in its `enabled` byte. Older firmware V1.21 and before reject newly saved projects. Keep a backup.
