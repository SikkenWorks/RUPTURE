# RUPTURE V1.58 SHAKE — release verification

This is the compiled V1.58 **SHAKE** release, not the previous source-only patch.

- SOUND SHAPE: volcano eruption shakes subtly by offsets of 1–2 pixels.
- ENV and LFO: the existing original 20-frame sprite sequences shake during opening/closing and continue with a small 3-phase UI jitter while keys remain held. LFO reverse and ENV closure are preserved.
- The original PNG sprite bytes (60/60) are **unchanged** from V1.57.
- Fast bit-packed UI row blit, no extra framebuffer and no per-pixel procedural redraw in the audio thread.
- Application binary: **312,556 bytes**, compared with 311,916 in V1.57 (delta **+640 bytes**). Free application capacity: **269,008 bytes**.
- Original critical audio and OTA files are byte-for-byte preserved, including `audio_core.c`, `native_fx.c`, `ota.c`, `usb.c`, `wave_store.c`.
- `FM-1_968` identity preserved.
- Protected 16 KiB bootheader, trusted OTA loader and flash tail at 0x93000..end are byte-identical to V1.57.

## Software tests

- JieLi pi32v2 native build / app entry / size: **PASS**.
- LFO+ENV note hold sustained three-state shake / release and reverse: **PASS**.
- SOUND SHAPE three-frame lava loop / return to idle: **PASS**.
- Native OTA valid app-write plus staging rejection and bootheader guard: **PASS**.
- Native rendered 240×240 screens for all three pages: **PASS**.
- `index.html` and `WebInstaller.html` embedded FWSC byte parity, SHA-256 and site scripts: **PASS**.
- `.fwsc` integrity CRC and product identity: **PASS**.
- Physical FM1 run, audio deadline/slack, stock V15 restore and alternate firmware transfers: **NOT TESTED**.
- Complete historical regression suite: **NOT RERUN**.

**Not audio-thread heavier:** no DSP, voice, ISR, oscillators, filter or audio buffer changes. UI redraw timing is not guaranteed identical; sustained LFO/ENV shaking adds periodic *UI-only* refreshes approximately every 110 ms.

SHA256 firmware: `1c4f829317017d81f108916cfad0afc98ffc2cbeee5ac838d07d66eb8cdb6d66`
