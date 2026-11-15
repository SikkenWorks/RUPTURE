# RUPTURE TEST RESULTS — V1.87

Date: 2026-11-11

## Scope
- Replace the BEND page mountain with the approved more realistic Mount-Everest-style eruption PNG sheet.
- Keep V1.86.1 K3 bend-curve presets intact.
- Keep the V1.86/V1.86.1 OTA/Web MIDI connection hotfix intact.
- Avoid any extra load on the audio engine.

## Result
- PASS: 8 offline-converted 224×30 1-bit BEND frames were integrated into `rupture_bend_sprites.h`.
- PASS: `bend_frame_00..07.png` exactly match the packed ROM sprite bytes.
- PASS: Audio / OTA / USB critical files remain byte-identical to V1.86.1.
- PASS: `rupture_ui.c` and `bringup.c` are unchanged from V1.86.1; only sprite assets/header were swapped.

## Critical unchanged-file parity
- PASS: `vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c` — SHA-256 `99e37d0564d5fd1bcb95ede763ed2c5218cd06f0c505d87f21e8b82c01feb645`
- PASS: `vendor/etch-v1.88-source/firmware/fm1-native/audio_target.c` — SHA-256 `b8dfa2ef0ced4be23a9f7e36a99d7327ec2867afde711f2df132c3ddc6071d2a`
- PASS: `vendor/etch-v1.88-source/firmware/fm1-native/usb.c` — SHA-256 `3e484359d55c68e7510b3b588315e1e781ca30d3946f7d0fea4288895cb6dbef`
- PASS: `vendor/etch-v1.88-source/firmware/fm1-native/ota.c` — SHA-256 `c77217b9a650beda39827a2754e76a8502ee97c8244ce07087e05c87dc9e34e6`
- PASS: `vendor/etch-v1.88-source/firmware/fm1-native/ota_target.c` — SHA-256 `1d9a9ceb0489d98211d9757eafcd9ed021b9f7d57efa4e0ef3631824f2a44c5a`
- PASS: `vendor/etch-v1.88-source/firmware/fm1-native/hal/fm1_isr.S` — SHA-256 `2aad84e24a51c79b187ef3faf9d1bae108b1389bf78659f6b02dbbd95d8a7d7b`
- PASS: `vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c` — SHA-256 `c2285cd6115fbadc6328f08a8a2182d71bae0eb5881de79eedb7ba54903cbf07`
- PASS: `vendor/etch-v1.88-source/firmware/fm1-native/bringup.c` — SHA-256 `e600e959a62e3a4540b69eafe5e17be8f3c438ea051f799a3b45a12cc6173a95`

## Changed files
- `vendor/etch-v1.88-source/firmware/fm1-native/rupture_bend_sprites.h`
- `vendor/etch-v1.88-source/firmware/fm1-native/bend_frames/bend_frame_00.png`
- `vendor/etch-v1.88-source/firmware/fm1-native/bend_frames/bend_frame_01.png`
- `vendor/etch-v1.88-source/firmware/fm1-native/bend_frames/bend_frame_02.png`
- `vendor/etch-v1.88-source/firmware/fm1-native/bend_frames/bend_frame_03.png`
- `vendor/etch-v1.88-source/firmware/fm1-native/bend_frames/bend_frame_04.png`
- `vendor/etch-v1.88-source/firmware/fm1-native/bend_frames/bend_frame_05.png`
- `vendor/etch-v1.88-source/firmware/fm1-native/bend_frames/bend_frame_06.png`
- `vendor/etch-v1.88-source/firmware/fm1-native/bend_frames/bend_frame_07.png`
- `vendor/etch-v1.88-source/firmware/fm1-native/bend_frames/BEND-CONTACT-SHEET.png`
- `assets/bend-v187-everest/*` (preview exports)
- `CHANGELOG.md`, `README.md`, `BEND-PATCH-NOTES.md`


## V1.87 actual release build and installer validation

- **PASS:** native JieLi/PI32V2 `--storage --updater --usb-audio-48k` FM1 build; application `359124` bytes, free `222440` bytes, entry point `0x02000120`.
- **PASS:** second independent compile was byte-identical (application SHA-256 `fed9777d06b5ce7531a7d4657f7f2a3c0665b05b5c05f3f81ce01344e2020807`).
- **PASS:** `.fwsc` package validates with exact FM1 identity `FM-1_968`, size `610086` bytes and SHA-256 `5d38295226a0ebc79336612b8a5e4e92bb9c82db2bda686979e7e3cf196b2de6`.
- **PASS:** original protected 16 KiB bootheader and 6930-byte trusted OTA loader are byte-identical to V1.86.1.
- **PASS:** WebInstaller and GitHub Pages index include exactly the release firmware (matching base64, size and SHA-256).
- **PASS:** the existing V1.86 connection-fix regression (both JEDEC IDs, unknown Windows MIDI ports, read-only identification) passes.
- **PASS:** five BEND K3 curve / CXS and SAW PCM regression tests pass; zero-bend matches prior PCM and sustained notes are not retriggered.
- **PASS:** 29 non-BEND native UI pages render byte-identically to V1.86.1 (screen version page excluded).
- **PASS:** actual 240×240 native BEND screen exported with CRT/VCR ON and OFF, showing the intended white mountain on a black background.
- **PASS:** 8 source PNGs match the packed ROM bytes, 224×30 1-bit each. Sprite count, blitter and frame size are unchanged.

**NOT TESTED:** any operation on a physical FM1, including real OTA flashing, LCD/audio timing, USB-C speaker plop, actual audio DMA LATE/GUARD and save/restart on the new UC25HQ80 flash chip. Results above are host/native-build validations, not an on-device guarantee.

## Notes
- This is a visual/UI-only asset refresh. No additional DSP, voices, filters, effects, storage writes or ISR work were added.
- OTA loader behavior, connection workaround logic and dual flash-ID support are inherited unchanged from V1.86.1.
- No physical FM1 flash/install test was performed in this environment.
