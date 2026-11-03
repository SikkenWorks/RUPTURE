# RUPTURE V1.56: technical release checks

## Scope

- LFO: approved top-down street / sinkhole sprite animation. 20 native 224x30 1-bit PNGs archived; frames 01–13 open, last note-off reverses those exact frames to 01. Frames 14–20 remain in source assets but aren't needed for the intentionally reversed LFO closure.
- ENV: separate approved brick-wall rupture; frames 01–13 open and frames 14–20 close on last note-off. Short notes choose an early close without jumping to the largest hole.
- All image data compiled as **33,600 static ROM bytes**. Runtime sprite copy is the same 840 bytes as PLAY DAM. No new framebuffer, heap allocation or UI-drawing operation in the audio ISR.
- The high-resolution AI-generated contact sheets were exported to native 224x30 1-bit PNGs once, offline. This conversion is not mathematically lossless relative to the large contact sheets, but the PNGs are *pixel-exact* in the firmware after conversion.

## Tests performed

| Check | Result |
|---|---|
| JieLi pi32v2 cross-build (`--storage --updater --usb-audio-48k`) | PASS |
| Application size and entry audit | PASS: 294,992 / 581,564 bytes, 286,572 bytes headroom |
| LFO PNG vs native renderer, screen x=8..231/y=50..79 | PASS 20/20 |
| ENV PNG vs native renderer, screen x=8..231/y=50..79 | PASS 20/20 |
| Unmodified pixels outside both sprite bars | PASS |
| LFO frame reversal, ENV closing frames, short note and retrigger | PASS |
| V1.55 PRESSURE PCM sweep / extreme parameters | PASS |
| V1.45 main oscillator/subs/screensaver loop | PASS |
| V1.48 SAW alias and neutral PCM | PASS |
| PLAY DAM V3 source PNG parity | PASS 20/20 |
| Native app-only OTA replay with V1.56 package, invalid staging rejection | PASS |
| Protected first 0x4000 flash bytes vs V1.55 | Byte-for-byte identical |
| Trusted OTA loader (`ota.bin`) vs V1.55 | Byte-for-byte identical |
| Flash content from 0x93000 to end vs V1.55 | Byte-for-byte identical |
| Critical `ota.c`, `usb.c`, DSP, PLAY DAM and saver source code vs V1.55 | Byte-for-byte identical |
| FWSC CRC / product identity FM-1_968 | PASS |
| `WebInstaller.html` and `index.html` embedded firmware | Byte-for-byte equal to .fwsc |
| Public page tests (including click/selection guard and success reconnect UI) | PASS |

V1.55 app: 260,908 bytes; V1.56 app: 294,992 bytes (**+34,084 bytes**). Binary increase is fixed sprite assets and UI control code. It is not increased DSP processing. These changes can still increase CPU time for UI redraws; physical CPU, ISR deadline, LCD refresh and audio glitch testing were **not performed**.

**OTA caution:** no stock V15 full-flash physical restore or custom cross-firmware switching was performed on an actual FM1 in this release. Native positive/negative OTA tests and byte parity are meaningful but do not establish a physical recovery guarantee. Back up data and confirm recovery before flashing.

Firmware package: `RUPTURE-V1.56.fwsc`; length 610,086 bytes; SHA256 `5de269b0d882680e1afebedb44ce7954cb5b39682e5b5d7c2956b93b9ee8df5b`.
