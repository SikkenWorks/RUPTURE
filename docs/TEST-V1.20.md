# RUPTURE V1.20 firmware verification

- Native JieLi crossbuild: PASS
- Previous regression suite: PASS
- 24 1-bit 240×240 real C-rendered animation frames: PASS
- Exact parity frame 24 vs final approved 240×240 image: PASS
- Protect firmware flash first 0x4000 bytes vs V1.19: byte-equal PASS
- Trusted OTA loader vs V1.19: byte-equal PASS
- FM-1_968 FWSC CRC: PASS
- WebInstaller firmware is byte-identical: PASS
- Application bytes: 194648; image slot headroom: 386916 bytes
- Added ROM art size: 7200 bytes; extra audio buffers: zero
- Firmware bytes: 610086
- FWSC SHA-256: 35e047e367c05cf708eddf20f5139ecbc9dd35bece85a76cabd4285a47d505c7
- Hardware boot / realtime interaction: NOT TESTED on FM1
