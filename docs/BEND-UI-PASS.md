# RUPTURE BEND | visual UI pass

This BEND UI source patch originated from V1.85.2 and is now included in the **native-compiled RUPTURE V1.86** release.

## BEND display (native 240 × 240 pixels)

- **FX short press:** normal FX screen (inherited behavior from BEND source patch)
- **FX hold (700 ms):** BEND screen
- **K1 / BEND:** momentary bend percentage (`0–100%`) and current pitch shift (`+0.00 ST` to the selected maximum)
- **K2 / RANGE:** five permanently visible targets, in an unmistakably selected tile: `1/4`, `1/2`, `FULL`, `1 1/2`, `DOUBLE`
- **Live meter:** actual current bend percentage, not a static setting or an FX wet/mix percentage
- **Status:** `IDLE`, `BENDING` or `RETURN` while the K1 bend is decaying after the last turn
- **Bottom help:** `STOP K1 = RELEASE`

The source contains **eight offline-generated, packed 1-bit PNG frames** of a mountain rising from the ground. The firmware blits those sprite bytes directly. It never draws the mountain geometry live or animates it inside the audio ISR. The original CRT/VCR optics still apply to BEND exactly as they apply to other screens.

## Scope of this pass

Only the BEND-specific layout and related UI status/publication were refined. The CXS/SAW generator, pitch factor, sample rate, reverb, other FX, MIDI, project format, flash writer, OTA loader, hardware ISR, Welcome flow, tutorial and other pages were not changed by this pass.

When BEND releases, the UI now only schedules new LCD work on a change of **one of the eight prepacked mountain frames**, rather than marking every 25 ms strength-decay tick as urgent. K1 input still triggers normal immediate repaint; the existing LCD budget can throttle noncritical art when audio deadlines are under pressure. This only changes **when BEND graphics repaint**, not pitch bend timing or strength.

## Included pixel-accurate renders

`assets/bend-ui-v186/` contains:

- 16 individual BEND screens at original **240 × 240** resolution (8 states each with CRT/VCR OFF and ON)
- 2 labelled contact sheets for quick comparison (nearest-neighbour enlarged; the underlying 240 × 240 exports are the exact renderer output)

`tests/render_bend_ui.c` invokes the same `rupture_ui.c` that is included in native firmware; it is not a manually replicated mockup.

## Validation performed

- GCC host syntax check passed for `bringup.c`, `audio_core.c` and `rupture_ui.c`.
- All 8 original 224 × 30 pixel, 1-bit mountain PNG frames matched their exact packed sprite bytes in `rupture_bend_sprites.h`.
- Host rendering: all 20 non-BEND screen samples compared with unmodified V1.85.2 baseline and were byte-identical with CRT/VCR ON.
- 16 BEND preview PNGs and 2 contact sheets checked for correct dimensions and state differences.
- No changes detected in audio FX, DMA ISR, USB, OTA, project codec or protected hardware ISR source.

**Not yet validated:** real FM1 display/frame timing, real hardware BEND feel, and speaker-pop-free startup after USB-C use. The V1.86 release includes a newly compiled `.fwsc` with BEND; confirm actual hardware behavior before public deployment.
