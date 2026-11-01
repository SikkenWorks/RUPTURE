# RUPTURE V1.44 — Official M-VAVE stock V15 return, native OTA audit

## User report and reproduction

On RUPTURE V1.43, returning to **official V15** from The Firmware Hub displays `OTA PREP FAILED / ERROR 3`. The original `FM-1_015` stock firmware file, recovered from the archived ETCH V0.33 test reference, is 699,956 bytes and has SHA-256 `db1642b2b6fa5c2cccb11ffd13878068bb28601678d3644049f99dc40e7edb8a`. It is **not redistributed** with RUPTURE.

The native OTA host-SysEx/C simulator, with this exact original `.fwsc` as the target and the RUPTURE V1.43 boot image as the installed device, reproduces the failure on unmodified V1.43: **`rc=-3` after 2 host reads, before any erase**. The actual stock UFW package has **eight entries** (`flash.bin`, `info.log`, `USR`, `isd_config.ini`, `ota.bin`, `script.ver`, `blimit.bin`, `tail.bin`). The V1.43 `ota_ufw()` routine rejected all entries except types 0 and 100 and enforced exactly two entries, which cannot parse official V15. This is a proven source-level cause of error 3. It was not related to exhausted storage or a failure to write flash.

## Source changes

1. **Explicit package family distinction:** accept only the old two-entry app-only layout or the precisely enumerated eight-entry original stock-V15 layout. Validate entry types, names, indexes, ordering, zero length only for the official empty `info.log`, declared ranges, pad alignment, CRC-protected header/list and non-overlapping payload positions. The auxiliary V15 records are **not executed** or staged; only `flash.bin` and `ota.bin` are used.
2. **Exact vendor stock identity:** if an eight-entry package contains an OTA loader other than the pinned original V15, reject it with `-20`, rather than attempting to treat it as an app-only firmware. For V15, verify the original OTA-loader full CRC and SHA-256 and the entire `flash.bin` CRC and SHA-256 *before* staging. Unknown/corrupt packages never reach the erase stage.
3. **Narrow bootheader exception:** the actual RUPTURE V1.43 packaged boot area and the exact official V15 boot area are identical at every byte except the four `uboot.boot` directory-CRC bytes at `0x20..0x23`. V1.44 allows **only that exact old/new four-byte pair**, with matching other 16,380 bytes, for the exact pinned official V15. Unknown boot variants (including unverified V16 boot areas) remain rejected `-5`. The stock restore still uses the verified original vendor full-flash loader; other firmware still uses the unchanged pinned community app-only loader.
4. No modifications to synthesis, effects, audio, UI, saved project format, reset/recovery code, first 16 KiB of packaged flash, or the packaged OTA loader binary.

## Testing (2026-11-01)

| Test | Result |
| --- | --- |
| V1.43 original native CHECK on original V15 | Reproduced error `-3` after 2 reads |
| V1.44 native CHECK on the **same exact original V15** | PASS, 2,546 host reads; staged original vendor loader; reached `ota_commit()` |
| RUPTURE bootheader vs stock, 4-byte metadata exception | PASS, full 16 KiB comparison |
| Unexpected boot code change (outside 4 allowed bytes) | PASS, rejects `-5`, no erase |
| Unexpected boot directory signature | PASS, rejects `-5`, no erase |
| Corrupted stock flash contents | PASS, rejects `-22` before erase |
| Corrupted UFW metadata | PASS, rejects `-2` before erase |
| Unknown nonblank OTA staging area | PASS, rejects `-15`, no erase |
| Existing two-entry custom-firmware OTA, blank staging | PASS, commit |
| Existing two-entry custom-firmware OTA, known old stage | PASS, commit and trusted loader |
| Unknown/broken residual OTA loader | PASS, refuse without erase |
| Cross-compiled JieLi pi32v2 app | PASS, 233,120 bytes of 581,564-byte slot |
| WebInstaller/index `.fwsc` embedded payload parity | PASS |
| Packaged boothead / app-only OTA loader vs V1.43 | PASS, byte-identical |
| Full historical regressions | See local `regression-v144.log`; not a physical-device substitute |
| Complete stock V15 **on-device** flash + power-cycle | NOT TESTED |
| On-device USB ROM rescue and OTA interruptions | NOT TESTED |

## Reproducing the native V15 test

Obtain the **official, unchanged** `FM-1.fwsc` V15 file from the M-VAVE download page and verify the SHA above. Provide a known matching `RUPTURE-V1.43.fwsc` from the last release (not contained in this versioned source ZIP) and run from the project root:

```sh
cc -O2 -std=c11 -Wno-unused-function tests/test_v144_stock_v15.c vendor/etch-v1.88-source/firmware/fm1-native/native_digest.c -o /tmp/rupture_stock_v15_test
/tmp/rupture_stock_v15_test /path/to/stock-V15.fwsc /path/to/RUPTURE-V1.43.fwsc
```

The harness simulates the native SysEx read protocol, validates the staging data and commit-request creation, and gates NOR writes. It **does not** emulate the stock loader's execution or prove that stock will boot on every physical unit.

## Firmware on already-affected FM1s

This fix requires V1.44 to be running on the device to change CHECK behavior. Earlier RUPTURE versions continue to enforce their original checks. If the installed RUPTURE accepts a compatible app-only V1.44 package, install that first, then retry the unchanged official V15 file. If no package can be installed, follow a verified USB ROM/boot rescue procedure with a backup, not arbitrary flash erasure or bypassing boot-region protections.

**Warning:** returning to stock may overwrite saved custom-firmware storage data. Export/backup work first. No physical stock-V15 flash has been performed by the assistant.
