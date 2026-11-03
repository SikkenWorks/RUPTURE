# RUPTURE V1.65 — Software Release Candidate Validation

**Status:** SOFTWARE RC — **NOT yet approved as a hardware-validated production release**.

## Scope and build

- Target: M-VAVE FM1, JieLi pi32v2 cross-compiler, `--storage --updater --usb-audio-48k --multi-project`.
- Firmware identity: `FM-1_968` (unchanged); project storage format and oscillator/DSP routes unchanged.
- Native app build: **PASS**, 318,676 bytes of 581,564 available; 262,888 bytes application headroom.
- Firmware package: **PASS**, 610,086 bytes, valid CRC/application image.
- Firmware SHA-256: `6f97b6e786eb989732cc9744e0d2c8893b008e187a86b5ff41eb5ed78d5978da`.
- UI version, installer and documentation now consistently say **V1.65**.

## Byte-level comparison to V1.64

- **164 native source files** match the pinned V1.64 hashes. `rupture_ui.c` changes only the displayed `SYSTEM INFO` version.
- The native application is the same size as V1.64 and differs at **one byte**, in its displayed `V1.65` version string.
- The packed `flash.bin` differs at **nine bytes**: eight application header/CRC bytes and one on-screen version byte. It is otherwise byte-identical to V1.64.
- **Protected first 16 KiB** of flash: byte-identical to V1.64.
- **Trusted `ota.bin` loader**: byte-identical to V1.64.
- Native updater/USB source: unchanged; firmware identity `FM-1_968` retained.

## Tests completed

- **PASS**: native build and application image validator.
- **PASS**: audio-first LCD scheduling and coarse battery status display.
- **PASS**: project unsaved mark, protected load confirmation and one-gesture undo.
- **PASS**: SCL labels, knobs, encoder acceleration and SYSTEM INFO rendering.
- **PASS**: first-run YES/NO paths, long SAVE format confirmation and failure/success messages.
- **PASS**: LFO / ENV shake, volcano 3-frame lava loop and note-off return, screensaver reverse loop.
- **PASS**: PRESSURE CXS DSP audible-output differentiation across 0..100 on two pitches (host PCM).
- **PASS**: CRT bypass and QR screen rendering.
- **PASS**: **60/60** original ENV, LFO, volcano source PNGs exactly match embedded ROM bitmaps.
- **PASS**: **180** animated UI render states match the expected source sprites under their intentional shake offsets; graphics outside the panel remain identical.
- **PASS**: native V1.43 app-only OTA replay against **V1.65 .fwsc**. Valid update accepted; recognized old loader reclaimed; corrupt and unknown staging rejected without unintended flash erase.
- **PASS**: firmware package matches embedded `WebInstaller.html` and `index.html` byte-for-byte, including SHA-256 and length. Installation result/reconnect handling retained.
- **PASS**: clean release root, README contains no version-by-version changelog, only one current root `.fwsc`.

Run `python3 tools/test_v165_rc_gate.py` in the project root for a reproducible host/software gate. You can also run `python3 tools/test_site.py` and `python3 tools/check_release_clean.py`.

## Incomplete, superseded or unavailable

- The historical `tests/run_tests.sh` was started and produced **35 lines of passing test output through the V1.17 FX memory test**, but exceeded the available runtime before completion. **It is not counted as a passed full suite**.
- The older V1.56/V1.57 screenshot comparison tests expected an unshifted bitmap. That became an invalid assumption after the intentional shake added in V1.58. These tests now delegate to the strict ROM-source and shaken-render comparison tests, and **PASS**.
- The official original V15 full-package replay (`tests/test_v144_stock_v15.c`) was **not performed** during this run because the exact required vendor V15 image was not part of the project/release inputs. App-only OTA replay does not substitute for this.
- No physical FM1 was connected. Real LCD timing, audio/USB DMA, persistent NOR writes, OTA to other firmware, return to V15, and any recovery device workflow remain **UNVERIFIED**.

## Release disposition

**RC HOLD** pending the hardware matrix in `docs/PHYSICAL-FM1-V1.65-ACCEPTANCE.md`. Do not claim V15 return-to-stock or cross-firmware recovery is proven until the corresponding physical tests pass. Back up projects and confirm a working rescue method before any OTA flashing.
