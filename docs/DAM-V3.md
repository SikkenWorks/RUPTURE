# RUPTURE V1.54: DAM V3 integration

## Sources
- `assets/dam-v3-frames/frames/01.png` through `20.png` are the supplied, pixel-perfect 224 × 30 1-bit V3 sprites. They are used *as-is*.
- `tools/compile_dam_v3.py` packs the exact byte values into `firmware/fm1-native/rupture_dam_sprites.h`.
- The native PLAY renderer copies one row of 28 bytes at a time into the existing canvas, at x=8, y=50. The whole frame blit is 840 bytes.

## Animation
- **Note on:** frame 2 starts the opening (frame 1 is idle); frames 2–10 advance every 65 ms.
- **Note held:** loops frames 11–13 only, at octave-dependent rates 250/200/140/110/85/65/50/40 ms across -2..+5.
- **Last note off:** streaming ends immediately. Recession uses matching frame 14–20, avoiding an artificial flood on a brief note.
- **Idle:** returns to frame 1 after closing; frame 20 visually equals frame 1.
- Hardware notes, sequencer and MIDI continue to use the existing voice-gate detection in `bringup.c`.

## Performance and safety
- The audio ISR, oscillator, filters, effects, USB/OTA and project-save code have **no source modifications**.
- 20 × 840 = **16,800 bytes ROM bitmap data**. Previous V1.53 had 12 × 840 = 10,080 bytes; incremental static assets +6,720 bytes.
- **No new framebuffer**, no interpolation, no floats, no bitmap decompression, no per-sample DSP.
- Per-frame pixel copy is exactly as V1.53; additional opening/closing frames result in a longer UI animation. No audio load increase is designed into the engine, but actual overall CPU/USB/LCD interrupt margins need testing on physical hardware.

## Software validation
- 20/20 original sprite PNGs byte-to-pixel-identical to the native framebuffer.
- Unrelated native UI pixels are unchanged across sprite frame switches.
- Note state progression, octave timing, short-key release and 3-frame looping have passing host tests.
- FM1 crossbuild, OTA package/header verification and selected audio regression tests passed. Hardware playback and DMA headroom not measured.
