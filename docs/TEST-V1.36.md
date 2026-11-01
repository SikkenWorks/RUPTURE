# RUPTURE V1.36 — Storage Recovery and AUTO LOAD PROJECT

## Scope

Native M-VAVE FM1 source update based on V1.35. No changes to the audio DSP, protected 16 KiB boot header, or trusted OTA loader.

## Root causes

1. **WIPE ALL**: the existing wipe operation requested erases through 0xDFFFF, but the native flash driver's normal permitted range ended earlier. The resulting access rejection surfaced as `SAVE FAILED`. A distinct, confirmation-gated wipe callback now reads, erases and verifies only documented user sectors 0x97000..0xDFFFF. Normal project program writes retain their original narrow allowlist. Firmware, OTA and boot areas are excluded.
2. **Project DELETE**: existing catalog edit rules rejected the active project, the last project and the shared wave-library owner. The final-project case now erases the two RUPTURE user catalog banks and can begin a fresh project afterward. If other saved projects still reference the owner's shared wave data, deletion is intentionally refused with **SHARED WAVES / LOCK**, rather than destroying dependent data or misleadingly reporting SAVE FAILED.
3. **Error recovery**: after an edit failure, the firmware checks that the committed catalog remains valid before clearing the transient failure status, preventing persistent save lockouts.
4. **AUTO LOAD PROJECT**: the new SETTINGS row opens a browser showing NEW and only currently saved project slots. NEW is default. A selected saved project is restored at boot; if missing, the device falls back to NEW. The selection is CRC-protected in the existing 8-byte device-global journal. Existing settings records remain readable.

## Verification results

| Verification | Result |
|---|---|
| JieLi pi32v2 native firmware build | PASS |
| V1.36 NOR flash emulation: new/legacy settings, AUTO LOAD choice, tear recovery, final project deletion and fresh SAVE | PASS |
| Legacy user-storage wipe and erase-bounds host tests | PASS |
| Historical regressions through V1.23 | PASS in timed initial run (the full uninterrupted script reached time limit) |
| Remaining V1.23–V1.34 tests run separately (sequencer, storage, EQ2, DSP/FX) | PASS |
| Firmware package CRC, protected 16 KiB bootheader parity and trusted OTA loader parity | PASS |
| Site `index.html`, standalone installer and `.fwsc` hash/size consistency | PASS |
| Physical FM1 cold boot, WIPE ALL and DELETE | NOT TESTED IN THIS ENVIRONMENT |

## Limitations and required real hardware checks

- **Back up user projects before flashing.** A destructive WIPE ALL is irreversible.
- Deleting the shared waveform-library owner while other projects depend on it is **not** supported on the memory-constrained FM1; this is deliberately blocked with a specific warning. Delete dependent projects first or back up and perform confirmed WIPE ALL.
- A last-project deletion also clears the device-global preferences stored alongside the project banks. AUTO LOAD returns to NEW.
- Native hardware flash erase timings, actual FM1 LED/UI input behavior, and cold-boot autoload require physical confirmation. Do not interpret successful host NOR simulations as physical verification.
- The full historical regression script exceeded the single-run time limit, but the remainder was run separately and passed.
