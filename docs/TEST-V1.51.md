# RUPTURE V1.51 — Build and Visual Check

- Native JieLi pi32v2 application build: **PASS** (255,744 bytes; 581,564-byte maximum)
- Candidate `.fwsc`: **PASS** (610,086 bytes, SHA-256 `2bdbd4c8377b9426a7eb3e716a9a597adcb82dcff0ec41c0d20710f1e7d72de9`)
- Identity: `FM-1_968` **unchanged**
- Protected 0x0000–0x3FFF bootheader and trusted ota.bin: **byte-identical to V1.50**
- Host UI CRT OFF: pixel-identical raw renderer, including splash and screensaver **PASS**
- Host UI CRT ON: tested on 8 screens; QR screen excludes distortion **PASS**
- Native settings NOR journal on/off, preserved by old volume APIs, after project save and compaction: **PASS**
- Existing screensaver forward/hold/reverse state unmodified. V1.50 Domtoren renderer retained.
- Browser and WebInstaller firmware byte parity: **PASS**
- JavaScript syntax: **PASS**
- No audio callback/DSP changes; no new fullscreen frame buffer, temporary row data 30 bytes, 1,200-byte read-only warp map.
- **Physical FM1 performance, display refresh, SPI timing, scanline legibility, and OTA install not tested.** This release requires hardware verification before treating it as safe or stable.
