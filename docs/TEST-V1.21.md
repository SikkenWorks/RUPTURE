# RUPTURE V1.21 SAVE routing validation

- Shared native `ru_save_destination()` routes SAVE from MAIN, SOUND SHAPE,
  SUB 1 and SUB 2 to the PROJECTS browser: PASS.
- SAVE from SEQ / SEQ SLOTS opens the separate SEQUENCE SLOTS menu: PASS.
- FX+SAVE chord cannot launch projects: PASS.
- NAME, GLOBAL, SAFE and unrelated pages cannot trigger project writes: PASS.
- Full RUPTURE host regression suite including FX, storage, ROM boot-art and
  project integrity: PASS.
- Native pi32v2 firmware crossbuild: PASS.
- CRC/OTA/header/installer parity: checked in release verification.
- Physical FM1 keyboard SAVE test: NOT YET PERFORMED.
