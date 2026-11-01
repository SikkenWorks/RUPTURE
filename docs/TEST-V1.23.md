# RUPTURE V1.23 validation

- Firmware: `RUPTURE-V1.23.fwsc`, 610086 bytes, SHA-256 `b2cba90c213f1bcf5ca2b9bf75f0d87649c4a4b665cf3d8a36817fab8e449798`.
- Native JieLi pi32v2 build: PASS (storage + updater + USB-audio 48k + multi-project).
- New real-time step sustain policy test: PASS, including stop-before-next-note, shrink, no new attack, original pitch/velocity unchanged.
- Project / eight sequence names + note duration project-encode/decode: PASS.
- Ten HELP cards, scrollable UI and positive/negative SAVE indicators: PASS.
- Existing FX, CXS, note memory, full storage emulation and 24-frame true 1-bit ROM intro host tests: PASS.
- New user-visible SAVE actions: FM1 source routing reviewed; **physical save/reboot tests NOT PERFORMED**. Host flash emulation validates lower-level storage, not every physical button event.
- Protected bootheader/OTA loader: validation against V1.20 in release build.
- UI: enlarged slogan in the regenerated exact 240x240 native bitmap.
