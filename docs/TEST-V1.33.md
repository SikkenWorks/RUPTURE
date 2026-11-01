# RUPTURE V1.33 – targeted firmware test results

**Firmware:** `RUPTURE-V1.33.fwsc` (SHA-256 `bf2d897af2fac889a89444e4093fc85defa6ee3cd5ea00a25569f2786d2e5a2c`); **identity:** `FM-1_968`.

## Implemented fixes

- The two small note-octave and HOLD values share the exact same x-coordinate and centered numeral column in the native 240×240 renderer.
- A new empty sequence slot no longer becomes falsely occupied immediately after the user enters its first name. The first subsequent SAVE initiates the real archived pattern commit; retry after a failed write retains the new-slot intent.
- Factory placeholders `ETCH24`–`ETCH31` and `ETCH 24`–`ETCH 31` do not prefill the name editor. Actual stored note data is not deleted or hidden solely because it has an old factory label.
- Saving now places a per-sequence LENGTH and BPM into existing compact patch fields tagged by `valid` bit 6; previously saved records remain readable. Loading fetches note/hold/lock data and restores LENGTH, BPM, rate, gate, swing and position before publishing to the audio engine.
- Project and sequence browsers display `ACTIVE` at the currently loaded slot regardless of the selected list row. Loading a new project clears its old active sequence identity.

## Completed tests

- JieLi pi32v2 native crossbuild and app validation: **PASS**. Application binary 215,464 bytes, headroom 366,100 bytes.
- Two separate sequence archives with distinct notes, LENGTH 4/8, BPM 92/167 and exact PJT33 encode/decode: **PASS**.
- Sequence duplication retaining notes, BPM/length marker: **PASS**.
- V1.28 user storage journal, NOR flash duplicate/rename/delete, legacy wipe boundaries: **PASS**.
- V1.30 legacy sequence listing and genuine filled patterns: **PASS**.
- V1.25–V1.30 source/UI targeted tests and V1.32 Utrecht-renderer tests: **PASS**.
- V1.33 source workflow and native 240×240 browser ACTIVE pixel tests: **PASS**.
- Original author QR: 841/841 modules, 21,025/21,025 pixels: **PASS**.
- Firmware package CRC/container verification, original first 16 KiB bootheader, identical OTA-loader: **PASS**.
- WebInstaller and `index.html` firmware-binary equality, SHA-256, document fallback, integrated website tests: **PASS**.
- Historical regression run passed stages through V1.24 then exceeded the test time limit. **Whole suite NOT fully completed** in a single run.

## Known limits

- **No physical FM1 test performed for V1.33 in this environment.** On-device save/restart, USB OTA, audio timing and buttons must be validated on the author's actual device.
- An earlier sequence that was displayed as “saved” but whose flash commit never completed has no saved note data to recover. This fix prevents the confirmed false-occupied first-save path; it cannot reconstruct unsaved notes.
- Older pre-V1.33 sequence records have no validated independent per-slot BPM or length. They load with the project's persisted timing as a fallback until saved again with V1.33.
- Make a backup before updating. Physical flash/recovery safety is not proved by host-only tests.
