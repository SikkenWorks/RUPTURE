# RUPTURE V1.57: SOUND SHAPE Volcano Integration — Test Report

## Implementation

- 20 original user-approved volcano illustrations exported once, offline, as 224×30 monochrome 1-bit PNGs; the firmware stores their exact exported pixels as a bitpacked ROM table (20 × 840 = 16,800 bytes).
- Native SOUND SHAPE bar (x=8..231, y=50..79) uses one fixed sprite memcpy-like pixel copy per UI redraw. The base PNG images are **not** decoded or scaled on the FM1.
- Note ON begins the cracking/eruption, three fixed eruption frames loop while notes are held, and Note OFF triggers recession and closing. A re-press during closing begins a new eruption.
- The same existing UI frame scheduler and UI buffer are reused. No new audio allocations, audio DSP operations, voice slots, sample-rate processing or additional full-screen framebuffers.

## Software tests

| Check | Outcome |
|---|---|
| JieLi native crossbuild `--storage --updater --usb-audio-48k` | PASS |
| App ELF format / XIP entry / slot size | PASS; 311,916 B app of 581,564 B; 269,648 B free |
| 20 PNGs to native renderer pixel-for-pixel | PASS 20/20 at 224×30 |
| UI frame comparison outside SOUND SHAPE waveform panel | PASS; identical outside panel |
| Volcano eruption and 3-frame loop | PASS |
| Release, short notes, retrigger; LFO reverse and ENV close | PASS |
| Existing V1.55 PRESSURE native host audio sweep | PASS |
| Existing V1.45 oscillator/screensaver reverse loop | PASS |
| Existing V1.48 SAW harmonic guard | PASS |
| Native OTA app-only replay with exact V1.57 .fwsc | PASS |
| Protected 16KiB bootheader, trusted OTA-loader, 0x93000+ regions vs V1.56 | byte-identical |
| 13 critical USB/OTA/DSP/persistence/older animation source files vs V1.56 | byte-identical |
| Website validity, embedded .fwsc parity / size / SHA256 | PASS |
| Existing installer success/reconnect UI and JS interaction guard | Present, website structural tests PASS |
| Complete historic regression suite | Not run |
| Physical FM1 UI/DMA/audio load testing | Not done |
| Real device cross-firmware changes or original stock V15 recovery | Not retested physically |

## Impact and cautions

Application size V1.56: 294,992 B; V1.57: 311,916 B; delta: +16,924 B. The extra size comes from fixed sprite artwork and UI handling, **not** a change to DSP code or buffers. Redrawing sprite frames still costs some display/UI CPU time. Without hardware load measurements, total CPU use or audio-deadline headroom cannot be claimed identical.

The existing OTA updater, V15 restore implementation and firmware identity `FM-1_968` are preserved. The package's protected bootheader and loader match V1.56, but successful native replay cannot guarantee physical recovery. Back up projects and verify recovery before flashing.

- Firmware: `610,086` bytes; SHA-256 `8393dbbbdef0a788f03f3878c20f8238bac8c71541238d754ff594618fcfb386`
- Application SHA-256 `a489d34af37f60adf4b2aae9aef515d47d19892ed22f1b70a2a4b1dad1e412ea`
