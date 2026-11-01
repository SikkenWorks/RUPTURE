# RUPTURE V1.31 — exact original QR correction

- Native JieLi pi32v2 crossbuild: PASS, 209600 byte application, 371964 bytes app headroom.
- Target firmware CRC, package integrity and original header / OTA loader: validation in release script.
- Original user-provided `qr-code.png`: unchanged, SHA-256 `10c58f4684cea5dcbe9597e5569657f5c63fa74e33a4102a24b40342e16ac9f2`. Kept as `assets/qr-code-original.png`.
- Firmware white-on-black QR bitmap equals source for **841/841** modules, as extracted from source raster.
- Native display renderer produces **21,025/21,025** exact 1-bit module pixels (5 × 5 per module).
- Native `REPORT A BUG` screen includes original QR with black quiet zone and label.
- Original QR destination decoded only after image inversion: `https://github.com/SikkenWorks/RUPTURE/issues`.
- DSP, project storage layout, boot and OTA implementation are unchanged.
- **On-device QR scanning is not hardware-validated.** Some QR camera readers do not support inverted white-on-black symbols. It must not be presented as hardware proven.
