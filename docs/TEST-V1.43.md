# RUPTURE V1.43 — Cross-Firmware OTA Prep Audit and Fix

## Report / reproduction
Two users report that **RUPTURE V1.42** prevents returning from RUPTURE to official FM-1 V15 using The Firmware Hub, displaying `OTA PREP FAILED`. The author reports that switching away works on their own FM1. No on-device OTA numeric error, bootsector dump, original V15 package, MIDI log or failing-device storage image was provided. Therefore the exact trigger on those units is not independently proven.

## Findings in the shipped V1.42 native C implementation

1. `firmware/fm1-native/ota.c`, `etch_ota_available()` used a strict allowlist for the reserved loader stage at `0xE0000..0xE4FFF`: **all erased**, 16-byte `ETCH-OTA-STAGE-1` ownership marker, or a loader with the exact ETCH/Felucca or official V15 body SHA-256. It *rejected a different, intact LOADER.BIN* left behind by a prior firmware as error `-15` **before staging the next loader**. The region is persistent across firmware updates, making this dependent on the unit's installation history. That is a concrete source-level bug and plausible reason two units differ from the author's unit.
2. For official V15 a pinned full-flash loader may rewrite flash `0x0000..0x3FFF`. V1.42 checked the target official payload but did **not** compare the target boothead with the boothead installed on the device. Differing stock V16/fork bootheads represent a separate cross-firmware hazard. Whether this affected the reporters is unknown.
3. RUPTURE's UPDATE screen showed only `OTA PREP FAILED`, hiding the native error code needed for diagnosis.

## Changes

- **Validated, bounded stage recovery:** A residual `LOADER.BIN` with a correct JieLi file-header CRC, name, destination (`0xE0000`), plausible encoded size and a valid inner loader header (`usb_hid_ota.bin`, expected execution address, length and header CRC) is recognized as **pre-existing OTA staging**, not user data. New firmware remains validated by the original header/entry CRC, complete payload CRC and SHA checks. The old executable is **never run**: app-only swaps stage the pinned trusted RUPTURE loader. Validating the old inner header is used only for classifying the staging location for replacement, not for trusting the old executable.
- **Unknown staging remains protected:** unexplained nonblank flash, corrupt JLFS headers or corrupt inner headers still return `-15` and perform no erase.
- **Official stock-V15 bootheader check:** only the exact existing pinned stock V15 full-flash payload and loader may be used, **and only if** the device's actual 16 KiB boothead is equal to the stock boothead (`-5` otherwise). This prevents a blind bootloader rewrite on a V16/different-base device. It is intentionally restrictive; affected owners may need USB ROM rescue or a separate hardware-validated transition. This is not an app-only check.
- **Diagnostic UI:** when CHECK fails the 240×240 native screen displays the error number. Error `15` denotes unrecognized staging; `5` denotes a protected boothead mismatch; `20` denotes stock package verification failure. Other errors are documented in native `ota.c`.
- No changes to synthesis, filters, DSP, sample storage, protected `.fwsc` bootheader, embedded trusted update loader or firmware package format.

## Tests run

| Test | Status |
| --- | --- |
| JieLi pi32v2 native storage/updater/UAC48 cross-build | PASS, app.bin 232700 / 581564 bytes |
| Native SysEx `.fwsc` host replay into empty NOR | PASS |
| Native SysEx `.fwsc` replay with recognizable residual loader | PASS, commit and pinned loader stage |
| Unknown nonblank OTA region left unchanged | PASS |
| Bad old loader inner header / corrupt file CRC | PASS, refuses before erase |
| Boothead exact comparison | PASS, matching accepted, mismatched refused |
| Firmware parser integrity, `.fwsc` header CRCs | PASS |
| V1.42 → V1.43 boothead first 16 KiB | PASS, byte-equal |
| V1.42 → V1.43 `.fwsc` OTA loader | PASS, byte-equal |
| Embedded standalone WebInstaller & index.html | PASS, byte-for-byte equal V1.43 `.fwsc` and SHA-256 |
| Host regression suite | PARTIAL, running through FX bypass without errors before execution limit |
| Exact official FM-1 V15 incoming package native replay | NOT RUN, original file unavailable in this environment |
| Real FM1 transfer from reported problematic devices | NOT RUN, external owners required |
| Actual V16 bootheader / V15 safe-downgrade compatibility | NOT VALIDATED |

## Already-affected V1.42 units

These units **still run V1.42's old CHECK code**. A new firmware cannot repair the older app before it installs. If every normal OTA attempt fails `OTA PREP FAILED`, the owner must use an independently validated rescue route to install a corrected image (for example The FM-1 Firmware Hub's documented **OCT− at power-on** ROM USB rescue). Confirm the exact FM1 chip and firmware image with that route and take a backup before proceeding. Never force a stock full-flash update across an unverified bootheader. Do not unplug while the updater is actively writing.

## Test artifacts

- `tests/test_v143_ota_reclaim.c`: native OTA host-SysEx + NOR regression fixture.
- Firmware: `RUPTURE-V1.43.fwsc`, normal identity `FM-1_968`.
- `RELEASE-MANIFEST.json`: current SHA-256 and package size.
- No physical test was conducted by the assistant.
