# RUPTURE V2 — SIGNAL BREACH

Boot/loading animation sprite assets for the M-VAVE FM1.

## Files

- `RUPTURE-V2-SIGNAL-BREACH-SPRITESHEET.png`: 4 columns × 3 rows, 960 × 720 pixels total.
- `frames/frame-01.png` through `frames/frame-12.png`: individual frames, each 240 × 240 pixels.
- `RUPTURE-V2-SIGNAL-BREACH-PREVIEW.gif`: animated timing reference.

All PNGs use a black-and-white 1-bit palette. Playback order is left-to-right, then top-to-bottom.

## Sequence

1. Black screen with a white scanline.
2. Signal interference begins.
3. SikkenWorks wordmark resolves from tracking noise.
4–5. SikkenWorks branding appears and stabilizes.
6–7. Fractures spread across the logo.
8–9. The image splits and glitches.
10. RUPTURE emerges through the interference.
11–12. RUPTURE V2 resolves and holds.

## Suggested timing

| Frame | Duration (ms) |
| --- | ---: |
| 01 | 90 |
| 02 | 70 |
| 03 | 100 |
| 04 | 180 |
| 05 | 130 |
| 06 | 100 |
| 07 | 80 |
| 08 | 70 |
| 09 | 60 |
| 10 | 90 |
| 11 | 110 |
| 12 | 800 |

These durations are a preview, not a forced startup delay. In firmware, show the completed logo only until initialization is finished, and then transition to the main interface. Do not block audio startup just to finish the animation.

No firmware code is changed in this asset package.
