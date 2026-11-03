# RUPTURE V1.64 — Software Validation

## Scope
Audio-first LCD scheduling and system battery-status wording; no changes to audio engine or OTA/recovery implementation.

## Passed (host/native)
- Native JieLi pi32v2 application compiled with storage, updater, 48 kHz USB audio and multi-project flags.
- App ELF conversion and image safety validation passed.
- LCD budget unit tests: immediate priority for input, page changes and notes; animation cadence adapts to audio pressure; idle refresh throttled; splash animates; duplicate full-screen paints removed; uint32 wrap-safe.
- System info host renderer: coarse battery bands, unavailable ADC, USB connected/disconnected distinct pixels.
- V1.63 project safety and UNDO tests passed.
- V1.61 knob/GLO tests, V1.60 welcome, V1.57 volcano, V1.58 panel shake and V1.55 CXS PRESSURE regressions passed.
- Protected 16 KiB bootheader and complete trusted OTA loader byte-identical to V1.63. Critical native audio, storage, USB and OTA sources also byte-identical.
- V1.43 native OTA app-only regression against the actual V1.64 `.fwsc`: valid update allowed, recognized residual loader reclaimed, corrupt loader/headers/unrecognized staging rejected without unintended flash erasure.
- WebInstaller base64 firmware matches RUPTURE-V1.64.fwsc exactly (SHA-256 + size).
- README contains no release history and only one active current firmware in project root.

## Not proven on physical hardware
- Sustained DSP/DMA stability, perceived latency, LED timing, real battery calibration and LCD frame timing.
- Actual cross-firmware migration and restoration of vendor V15. Firmware safety tests are not a physical recovery guarantee.
