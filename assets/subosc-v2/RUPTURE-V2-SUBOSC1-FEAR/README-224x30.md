# RUPTURE V2 — SUB OSC 1 FEAR

Native-size black-and-white 1-bit animation for the 224 × 30 pixel animation viewport.

## Included

- `RUPTURE-SUBOSC1-FEAR-SPRITESHEET-224x30.png` — 4 columns × 4 rows; total image size 896 × 120 pixels.
- `frames-224x30/frame-01.png` ... `frame-16.png` — sixteen individual 224 × 30 frames.
- `RUPTURE-SUBOSC1-EYE-TREMOR-LOOP.gif` — 4-frame eye-tremor preview, enlarged 4× using nearest-neighbor scaling for inspection.

## Suggested playback behavior

- **Idle:** show frame 01 without animation.
- **Note-on:** play frames 02–12 once, in order, to turn the face toward the viewer.
- **While a note is held:** loop frames 13–16 for the frightened eye tremor.
- **Note-off:** optionally reverse the turn back to frame 01; only run if it fits the existing note-off and UI behavior.

Sprite order in the sheet is left-to-right and then top-to-bottom. The graphics are intended to be drawn by the UI without adding any audio processing. This package contains assets only; it does not modify firmware.

Each packed 1-bit frame requires 840 bytes of uncompressed bitmap storage (224 × 30 ÷ 8), or 13,440 bytes for all sixteen frames.
