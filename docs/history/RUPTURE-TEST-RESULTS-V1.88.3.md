# RUPTURE V1.88.3 – ENV Animation and OTA Regression Report

Date: 2026-11-13

## Implemented

- Native ENV ROM sheet: **20 frames**, 224 × 30 pixels, 1-bit monochrome.
- Frames **1–12** introduce the wall fracture and smoke eruption.
- Frames **13–20** let the plumes settle completely and leave the exposed hole.
- While a note is held, frame 20 remains on the screen: no static smoke plume.
- On note release, the firmware counts **backward from the currently displayed frame** to frame 1, with no jump or separate pre-rendered closing artwork. Early release and retrigger are handled.
- The hold-frame ROM bank contains eight exact copies of ENV frames 13–20 for the existing UI path, avoiding animation discontinuities.
- Original LFO, SHAPE, BEND, audio, MIDI, flash storage and FX paths are preserved.

## Cross-build

Build flags: `--storage --updater --usb-audio-48k`

| Check | Result |
|---|---|
| Native FM1 cross-build | **PASS** |
| New app size | 386,248 bytes |
| Previous V1.88.2 app size | 386,156 bytes |
| Size increase | 92 bytes |
| FWSC size | 610,086 bytes |
| App SHA-256 | `1e77d9dd1436a68bb9cd378d9fd79cf64e2ec1b7ad60f78e8881e3bfeb84bb27` |
| FWSC SHA-256 | `3bc8d8fc365a664bd73e07562e31f9b68f9c75f633991945307ab2809eb34451` |

## OTA and flash safety

| Check | Result |
|---|---|
| Original trusted `ota.bin` V1.88.2 vs V1.88.3 | **PASS: byte-identical** |
| Protected first `0x4000` bytes (16 KiB) | **PASS: byte-identical** |
| WebInstaller firmware embed, SHA and byte count | **PASS** |
| GitHub Pages `index.html` firmware embed | **PASS** |
| Web MIDI connection/identity JS vs V1.88.2 | **PASS: only version/firmware/hash/length changed** |
| Puya P25Q80 JEDEC `0x856014` | **PASS: still recognized** |
| UC25HQ80 JEDEC `0xB36014` | **PASS: still recognized** |
| 19 DSP/MIDI/flash/OTA/BEND native files vs V1.88.2 | **PASS: exact source SHA-256 parity** |
| FWSC structural and CRC verification | **PASS** |

Trusted OTA loader SHA-256: `d71d1b7c12adede951605e758a3580dacccedc427d49e1fceaf51decccc68a4b`

Protected bootheader SHA-256: `a65e0e1a2fad1db943c00b529b45553c524257146145db61b69628cefcf2b2c9`

## Tests

- **PASS:** V1.88.3 ENV native C animation tests; full-frame and hold-frame ROM parity; settle-to-hole; reverse for short/long notes; retrigger.
- **PASS:** PNG-to-ROM parity: 60/60 PNGs for ENV/LFO native sprites, no resampling.
- **PASS:** Hold-to-full sprite parity: all eight ENV settling sprites; original eight LFO debris sprites.
- **PASS:** Earlier V1.56/1.57/1.58/1.66/1.70 animation/regression tests.
- **PASS:** V1.88.1 BEND storyboard test.
- **PASS:** Flash JEDEC test and V1.86 Web MIDI generic-port/identity regression.
- **PASS:** V1.88.3 OTA package/integrity, site build and static-site tests.

The historical catch-all `tests/run_tests.sh` was also attempted. It reaches V1.27 and halts at `test_v127_native_ui.c:19` (an obsolete coordinate assertion for the current settings UI). The V1.88.3 edit to `rupture_ui.c` changes only an ENV comment and the ABOUT version string, so this screen assertion is unrelated to the new animation. Do not interpret the interrupted catch-all run as a full-suite pass.

## Remaining on-device validation

**No physical FM1 was connected during this build.** The cross-build and host tests cannot prove actual OTA transfer, Flash writing, display refresh, timing, or audio performance on hardware. Before a public firmware release, make a project backup, have a verified FM1 recovery method ready, test the OTA transition and all animation phases on a physical unit (including recent UC25HQ80 hardware), and verify persistence across reboot.

The firmware is an **RC pending hardware validation**. An intact OTA loader and protected bootheader lower one class of regression risk but do not guarantee a trouble-free flash on all devices.
