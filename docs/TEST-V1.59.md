# RUPTURE V1.59: First-Run Welcome Verification

## Verified in software

- Native JieLi pi32v2 build succeeds with storage, updater, USB 48K and multi-project enabled: **315,256-byte app**, headroom **266,308 bytes**.
- Welcome state-machine test: BACKUP YES/NO, **NO :(** and continue, FORMAT NO and confirmation cancellation, 3-second physical SAVE hold before any format request, CRT ON/OFF and final CONTINUE.
- The native 240×240 monochrome renderer produced nine complete welcome states without a second framebuffer; see `assets/welcome-previews/`.
- Simulated SPI NOR 1 MiB storage test: first-run completion marker survives older global-settings APIs, project bank changes, 28-entry journal rollover and a failed write. Validated that old records remain readable. Optional initialization on blank storage writes only the designated owned 0xB4000–0xDC000 storage banks, and a second initialization refuses to overwrite a valid project store.
- Native OTA host test with `RUPTURE-V1.59.fwsc`: valid app-only update, recognized residual loader, trusted pinned loader, CRC corruption, unknown staging and mismatched boothead all handled as expected.
- Protected first 16 KiB boot area, complete OTA loader and flash outside application are byte-identical to V1.58. `ota.c`, `usb.c`, `audio_core.c`, `native_cxs_q15.c` and `native_fx.c` are byte-identical.
- Existing LFO/ENV/volcano shake, CXS PRESSURE 0–100, oscillator/saver forward-reverse and SAVE navigation regression tests pass.
- WebInstaller and website embed the exact same firmware bytes and SHA-256 as the `.fwsc` file. Text-selection and context-menu guard and reconnect/100% installer handling remain.

## Safety rules

- No storage writes while presenting questions. A format can be requested only from FORMAT YES + uninterrupted SAVE hold for three seconds.
- Valid RUPTURE project storage is deliberately not erased or rewritten by wizard format confirmation.
- For foreign or empty storage, initialization remains subject to existing recovery guards and the FM1's bounded flash-write path.
- First-run preference is saved only if RUPTURE's compatible device storage is available. If storage is foreign and formatting is declined, it may not persist across reboots. The wizard still allows users to continue.
- No new flash layout or project serialization format, DMA callback, synth code or audio buffer was added.

## Not verified on physical hardware

- Physical FM1 display contrast/brightness and panel input mapping during the first-run overlay.
- Flash formatting/initialization on a real FM1, boot-cycle persistence after a first-run completion and hardware behavior with foreign storage.
- Actual cross-firmware transitions and returning to official V15. The updater code and trusted loader are preserved, but cross-device compatibility is not guaranteed by software tests alone.
- Worst-case concurrent screen redraw and audio DMA margin.
