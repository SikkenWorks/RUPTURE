# RUPTURE V1.85.2 — UC25HQ80 / Puya flash compatibility audit

## Request and baseline

- Target: M-VAVE FM-1_968, based on complete RUPTURE V1.85.1 sources.
- Puya P25Q80: JEDEC `0x856014`, 1 MiB SPI NOR.
- UCUN UC25HQ80: JEDEC `0xB36014`, 1 MiB SPI NOR.
- Public reference: Felucca issue #320. The Felucca maintainer explicitly says its app-only OTA loader has no separate JEDEC check; an app-side gate formerly blocked updates there.
- **Baseline observation:** this particular RUPTURE V1.85.1 app had a JEDEC read routine with an obsolete Puya-only *comment*, but had no live Puya-only allowlist or `flash_ok` gate. The trusted OTA loader is included as a compiled, audited binary, without its source. It was deliberately not hex-patched or replaced.

## V1.85.2 change set

- New `hal/fm1_flash_id.h`: `FL_JEDEC_OK(id)` returns true only for `0x856014` or `0xB36014`.
- `hal/fm1_flash.h`: include the common ID header, update obsolete JEDEC comment.
- `wave_store_target.c`: cache the ID once during read-only boot probing before audio starts. Gate explicit project erase, page program, and wipe erase against a readable unsupported ID. The ID `0` from an unavailable probe leaves legacy V1.85.1 write behavior unchanged; this is **not** evidence of a supported device.
- `ota_target.c`: apply the same compatible-ID write policy to OTA-staging erase/page writes.
- `ota.c`: include cached flash JEDEC in SysEx identity reply bytes 24–26; the existing identity string and protocol are untouched.
- No changes to audio PCM/DSP, FX, sequencer, UI, hardware LED/PC6, project codec, bootheader, trusted loader, storage partitions or timing constants.

## Source tests

- `tests/test_v1852_flash_chip.c`: checks both accepted JEDEC IDs; unknown IDs refused; a zero/unavailable probe preserves legacy behavior; MIDI-ID bytes correctly encode both IDs.
- `tests/test_v1852_flash_chip.py`: verifies boot probe + all three storage and two OTA write safety gates are present and connected, and preexisting audio/FX/project codec/HAL ISR/trusted loader source files are byte-identical to V1.85.1. Only four existing native files changed, plus a new header.
- `tests/test_v185_realtime.sh`: PASS randomized panel MIDI parity for 120,000 states; bounded USB MIDI FIFO; native UI sanity.
- `tests/test_v1851_length_audio.sh`: PASS true audio LENGTH 16→4→16, preservation of existing notes, normal lengths 8–128.
- `tools/test_site.py`, `tests/test_website_release.py`: PASS firmware bytes/hash in WebInstaller and GitHub Pages index, metadata, documentation and unique page IDs.

## Native firmware and OTA integrity

- Production build flags: `--storage --updater --usb-audio-48k`, JieLi pi32v2 toolchain.
- FM1 application: **348,560 bytes**, flash app slot **581,564 bytes**, free space **233,004 bytes**.
- Second native build: byte-identical.
- Application SHA-256: `b2d517230e6896d10e53182ce1a085b17abe0621758fba634ddd3b087d4a1bbb`.
- `.fwsc` package: **610,086 bytes**; SHA-256 `c0de3025f84b0ba371fa6f7ca69b32bb0be9534daa404b4dcd89f28698ce6856`.
- Protected 16 KiB bootheader SHA-256 `a65e0e1a2fad1db943c00b529b45553c524257146145db61b69628cefcf2b2c9`, **identical to V1.85.1**.
- Original packaged `ota.bin` (6930 bytes) SHA-256 `d71d1b7c12adede951605e758a3580dacccedc427d49e1fceaf51decccc68a4b`, **identical to V1.85.1**.
- The separate RUPTURE trusted app-only loader compiled into the app is also unchanged, SHA-256 `d0b1a05c63030b58e5be06b93751e6cc375d3d34eec584109f97fd31f0a42a78` (6898 bytes).
- The `.fwsc` was parsed and passed the repository package verification routines.

## Not proven / release cautions

- **Not physically tested on a new-chip FM1.** A passing build/host test does not prove an actual cold-power-cycle save, OTA handoff, or recovery works on 0xB36014 hardware.
- Some older third-party firmwares may prevent the update from ever starting; V1.85.2 cannot update a blocker it has not yet replaced. Use only a known-good stock/ROM install and recovery method, with a full flash backup.
- Neither the vendor bootloader nor the trusted app-only OTA writer was rebuilt. Because the trusted loader has no independent Puya-only check according to its upstream maintainer and no source is included in this project, editing the audited loader would introduce an untested brick risk.
- No claim that the previous speaker pop or DMA LATE/GUARD reports are resolved by the flash-ID change.

## Suggested hardware acceptance (new flash chip)

1. Back up the full 1 MiB flash with an independent tool and confirm the device reports JEDEC `0xB36014`.
2. Install using a known working recovery/stock-to-custom route. Do not force unknown bootloader images.
3. Confirm the on-device synth starts and saves a named test project.
4. Disconnect USB, power off completely, power on and reopen that project. Repeat with global settings.
5. With a recovery route available, test one OTA update and verify the project survives.
6. Perform the same checks with an original Puya 0x856014 unit.
