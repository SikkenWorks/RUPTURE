# RUPTURE V1.54 — Technical validation

- Native JieLi pi32v2 crossbuild with persistent storage, OTA and USB 48 kHz flags: **PASS**.
- App binary: **260872 bytes**, SHA-256 `63ad8a0ca0581b4703f5a791f789da48e15ccdf1db5128bc85cc549489b779ee`; V1.53 app was 254128 bytes; delta **+6744 bytes**.
- App ROM headroom: **320692 bytes** (cap 581564).
- Exact original PNG sprites via native screenshot parity: **20/20 PASS**.
- UI animation C test: **PASS**, three-frame leak loop and OCT rates.
- Native screensaver forward/hold/reverse test: **PASS**.
- V1.48 saw guard/neutral PCM: **PASS**.
- V1.39 EQ2 unity pass-through: **PASS**.
- Native V1.43 app-only OTA replay using this .fwsc: **PASS**.
- New package: 610086 bytes SHA-256 `59d658030e54a2800a74796c48484d2f0b16f100154f3ab2f13e0a39ab758229`, firmware ID `FM-1_968`, CRC verifier **PASS**.
- Bootheader (first 0x4000 bytes), OTA loader (`ota.bin`): **identical to V1.53**.
- Audio/DSP and IRQ source files unchanged from V1.53: **confirmed SHA-256 parity**.
- WebInstaller standalone and website package bytes match the .fwsc; JavaScript syntax and site tests **PASS**.

## Remaining limits
- Full historical native regression suite was **not performed**.
- Physical FM1 flashing, MIDI reconnect, LCD timing, 8/16-bit display paths, measured audio IRQ headroom and DMA-stability with the longer animation **not yet tested**.
- The UI has more animation stages but unchanged per-frame transfer. Overall CPU impact may be nonzero; no audio DSP load has been added.
