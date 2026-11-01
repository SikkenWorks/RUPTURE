# RUPTURE V1.27 — build and validation report

## Verified in this environment

- JieLi pi32v2 native firmware app built with storage, updater, USB 48 kHz audio and twelve-project support.
- Built app: **199836 bytes**; SHA-256 `80e948356373845f223fc79cabb8f01450975cf7275f8f9e16ae32c2d0257f77`.
- FWSC: **610086 bytes**; SHA-256 `fd5f3ac38b9bf6a18f60e5b24da4223de7608f5ed44c731a912a9150511e4a24`. Identity stays `FM-1_968`.
- The OTA `ota.bin` body and protected 16 KiB flash head must be checked byte-for-byte against the pinned Felucca/ETCH baseline by the release script.
- 4-step and 8-step sequencer grids show only the defined active steps, verified on native C-generated 240×240 images.
- GLO cards and SETTINGS / MASTER VOLUME / AUTO SAVE UI render in the native FM1 renderer.
- UI-only crack screensaver renders progressively and starts with all-black; no changes to native_cxs_q15.c, audio_core.c or native_fx.c.
- Legacy global-volume NOR record, new AUTO SAVE record (OFF/ON), torn write, cross-project A/B transfer and 28-entry compaction tested against emulated NOR flash.
- Source policy regression confirms AUTO SAVE is disabled by default and ALGORITHM has no generic parameter fallback.
- README is product documentation with attribution, with version history exclusively in CHANGELOG.md.

## Not established

- The newly compiled V1.27 firmware has **not** been run on physical FM1 hardware here. The author reports ongoing real FM1 testing of RUPTURE generally; that does not certify the new binary.
- The 30-second wake-up behavior, OTA transition, screen draw scheduling, flash writes and USB power/current hardware signals require checking with the real device.
- Loader-side display progress during the actual flash WRITE remains unavailable: the trusted loader is deliberately unchanged. WebInstaller displays its genuine transfer progress.

Back up projects, ensure a working recovery path and do not claim that emulator results prove physical FM1 safety.
