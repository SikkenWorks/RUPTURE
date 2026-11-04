# RUPTURE V1.82 — First-boot migration validation

## Changes
- Existing `welcome_done` is still readable, but alone does not mean the V1.82 first-boot wizard was completed.
- New version-specific CRC-protected marker uses previously unused magic ranges of the existing 8-byte settings journal (`0x6000` and `0x6200`), without increasing record size or changing project payload/storage boundaries.
- On first boot from earlier firmware, the complete Welcome wizard opens again. Finishing the wizard commits the new marker; normal subsequent boots skip the wizard.
- Existing valid RUPTURE project storage cannot be formatted by the Welcome action. Explicit user confirmation is required for initializing eligible empty/foreign storage. Other project contents and settings are not reset.

## Tests carried out
- Native JieLi pi32v2 cross-compile with persistent store, updater, 48 kHz USB-audio and multi-project: PASS.
- Native app image validated at 347,984 bytes, 233,580 bytes spare: PASS.
- New V1.82 NOR emulator test: old completed marker -> new wizard needed; save completion -> second boot skips; two saved projects and settings retained; torn write fails closed: PASS.
- Existing V1.59 settings, bank-swap, journal compaction and power-loss tests: PASS.
- Firmware package checks: UFW verification, protected 16 KiB boot-header and trusted OTA binary equal to V1.81, app content matches native build: PASS.
- Physical FM1 first-run, formatting, V15 roundtrip and OTA write: NOT TESTED.
- Full historical audio/GUI regressions: NOT RUN in this iteration. Synth and FX sources unchanged; don't interpret this as full hardware validation.

## Limitation: V15 roundtrip to the same firmware version
If V15 preserves the RUPTURE settings journal and the user installs exactly the same V1.82 binary again, the FM1 cannot reliably infer that it temporarily ran stock firmware. Version-marker detection cannot distinguish that sequence from a normal restart. Forcing the wizard after every restart would be misleading and disrupt ordinary use. A safe explicit re-run path would require a separate user action or host-side installation marker, with documented hardware compatibility.

## Safety
The original OTA C module, protected flash bootheader and trusted loader are unchanged. The setup never silently formats existing projects. Always keep a backup and a tested hardware recovery method for experimental flashing.
