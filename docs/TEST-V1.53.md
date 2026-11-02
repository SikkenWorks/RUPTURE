# RUPTURE V1.53: Default CRT OFF — Test report

- FM1 pi32v2 crossbuild: PASS, native app 254,128 bytes; headroom 327,436 bytes (build audit).
- FWSC packaging and identity: PASS, 610,086 bytes, FM-1_968.
- Protected 16 KiB firmware boothead and OTA loader: byte-identical to V1.52.
- New/no-saved-CRT-settings boot: OFF; V1.53 persisted global settings: OFF. Legacy explicit ON remains ON and must be switched OFF manually if desired.
- Existing 8-byte CRC NOR settings record: unchanged layout; first-boot, save, log compaction and project-retention test PASS.
- CRT host renderer test: PASS, CRT OFF retains original binary output; optional CRT ON retains V1.52 geometry; QR exempt.
- DAM sprite parity: 12/12 original source frames pixel-identical in normal CRT OFF display.
- Screensaver forward/hold/reverse/loop: PASS in host test.
- Website and standalone WebInstaller JavaScript syntax: PASS; standalone firmware equals the new .fwsc. The index.html is based on the newest user-approved V1.52 installer, including reconnect and visual completion fixes.
- Full historical regression suite: NOT executed in this iteration.
- Physical FM1 update, performance and screen verification: NOT executed. This .fwsc is not a guarantee of safe installation; use backups and a verified recovery route.
