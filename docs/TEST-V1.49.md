# RUPTURE V1.49 — Native DAM sprite integration test

## Delivered

- 12 exact 1-bit PNG sources from the user's uploaded `frames.zip` preserved under `assets/dam-frames/`.
- `rupture_dam_sprites.h`: 12 x 224x30 pixel sprites, 10,080 bytes of static ROM pixel bits.
- `rupture_dam_anim.h`: display-only idle/open/leak/close state machine; 3-frame liquid loop.
- PLAY renderer uses byte-aligned 28-byte rows at (8,50) on the existing 240x240 1-bit UI canvas, with **zero new framebuffer allocations**. No procedural image redraw.
- Timing by **displayed** octave -2..+5: 270, 220, 165, 115, 75, 55, 45, 35ms per liquid frame, quantized to the 25ms input poll. MIDI, physical keys and active sequencer/arp voice gates all count; release of the final held gate starts closing immediately. Re-trigger during closing reopens.

## Measured software checks

| Check | Result |
|---|---|
| Exact 1-bit native render vs original 12 PNGs, 224x30 at x=8 y=50 | 12/12 PASS |
| Unchanged UI pixels outside sprite rectangle across all twelve frames | PASS |
| Open, three-frame cycle, stop/close, short-note closing, re-trigger, octave speed mapping | PASS |
| JieLi FM1 pi32v2 application crossbuild, 48k USB audio, storage, updater | PASS |
| ELF/app validator and _start/address/size audit | PASS |
| HAL/boot-reference 19-source integrity audit | PASS |
| Native RUPTURE UI/octave regression | PASS |
| V1.48 SAVE routing, all 49 screen IDs | PASS |
| V1.48 SAW anti-alias neutral parity and stress | PASS |
| V1.47 sustained SAW shaping | PASS |
| V1.46 SAW four controls | PASS |
| V1.45 suboscillator + screensaver regression | PASS |
| WebInstaller and public index.html embed exactly the released .fwsc bytes | PASS |
| Firmware .fwsc verification and app checksum | PASS |
| Existing V1.48 bootheader and OTA-loader preserved byte-for-byte | PASS |
| On-device FM1 audio/DMA timing and update | NOT TESTED |

## Build measurements

- App binary: **252,700 bytes** vs V1.48 **242,336 bytes**, growth **10,364 bytes** (10,080 bytes original 1-bit sprites, small UI state handling changes).
- App image limit: 581,564 bytes; remaining headroom **328,864 bytes**.
- `RUPTURE-V1.49.fwsc`: **610,086 bytes**, SHA-256 `4c599eccfaf67004e0a109d5b34d8c1a1b561f8088d14cac4f8e1cd5d5a148cb`.
- Protected flash header (0x0000–0x3fff) and packaged OTA loader exactly match original V1.48.
- Audio ISR, synth/FX DSP, storage layout, project format and the actual OTA runtime source remain unchanged.
- **Unmeasured:** real LCD DMA timing at the highest octaves and audio interrupt slack while held notes + USB audio + effects are active. The sprite implementation reduces drawing work but frequent changing LCD bands still consume bandwidth; no zero-cost performance claim is made.

## Reproduction

```sh
python tests/test_dam_sprites.py
gcc -std=c11 -O2 tests/test_dam_anim.c -o /tmp/dam && /tmp/dam
JIELI_TOOLCHAIN=/path/to/jieli/toolchain python vendor/etch-v1.88-source/firmware/fm1-native/build.py --storage --updater --usb-audio-48k
python tools/package_firmware.py
python tools/build_site.py
python tools/test_site.py
python tests/test_website_release.py
```

**Physical FM1 install risk:** Keep a backup of projects and a working restore route. The `.fwsc` file and WebInstaller are structurally validated, not confirmed by a real FM1 install in this session.
