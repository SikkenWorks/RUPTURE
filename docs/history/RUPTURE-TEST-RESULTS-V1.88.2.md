# RUPTURE V1.88.2 — All BEND animations: 1:1 crop fix

Date: 2026-11-11
Base: V1.88.1 user-approved poster release

## Root cause
The V1.88.1 sprite extraction tool resized poster image cells independently along X and Y (approximately 156 × 90 to 224 × 30), changing their aspect ratios and flattening their subjects.

## Fix
- **40/40 frames** use exactly 1:1 source pixel cropping, without interpolation or resizing.
- Per-frame Y camera windows select the most relevant subject details from original poster cells.
- Source crop width is 155–156 pixels, preserved as-is and centered in the 224 × 30 native viewport. Black side margins provide the remainder; tall art is intentionally clipped.
- Bit conversion uses the same threshold **112** and the same ROM bit order.
- Native sprites remain exactly **33,600 ROM bytes** (5 × 8 × 30 × 28).
- 4 animations keep their previous shake; S-CURVE close-up stays still. Existing storyboard/pitch/K1/K2/K3 logic untouched.
- Physical FM1 screen remains 240 × 240, sprite panel remains 224 × 30; no other UI layout changes.

## Tests
- PASS: 40 native PNGs are exact 1:1 cropped source pixels after 1-bit threshold; all map bit-for-bit to the compiled ROM sprite header.
- PASS: all 40 new frames differ from the aspect-distorted V1.88.1 output.
- PASS: 80 native 240 × 240 UI previews (5 curves × 8 frames × CRT on/off).
- PASS: four shaking eruption curves; S-CURVE ship close-up remains phase-independent.
- PASS: native FM1 cross-compilation, twice with identical app SHA-256.
- PASS: CXS and SAW bend sustained audio tests; five K3 curves; no retrigger; neutral PCM unchanged.
- PASS: V1.88.1 storyboard and reverse/release logic.
- PASS: Web MIDI FM1 identity on Puya `0x856014` and UC25HQ80 `0xB36014`, unknown port-name read-only probes.
- PASS: native `.fwsc` integrity and index/WebInstaller embedded package parity.
- PASS: all 18 critical audio, DSP, MIDI, flash, ISR, and OTA source files byte-identical to V1.88.1.
- PASS: OTA loader and 16 KiB protected bootheader byte-identical to V1.88.1.

## Build and integrity
- App bytes: **386156**; application headroom: **195408** bytes.
- App SHA-256: `7e88cd529979fed1f26aaca7da93f61eec5fc734d002f78c75ec4ac3939fdabe`
- FM1 `.fwsc` bytes: **610086**
- FM1 `.fwsc` SHA-256: `09b45e1c99c5ab51b27556286288a16460f6b133c49d03e2a31c2e6ed046296b`
- Original trusted OTA loader SHA-256: `d71d1b7c12adede951605e758a3580dacccedc427d49e1fceaf51decccc68a4b`
- Original protected bootheader SHA-256: `a65e0e1a2fad1db943c00b529b45553c524257146145db61b69628cefcf2b2c9`
- Poster source SHA-256: `a3f13d341145dc9e4647f40868293cf638465f59dfcb4294ca589505bca80387`
- New 33,600-byte native 1-bit sprite bank SHA-256: `56db8951654794af2a24216db72c5ee2aeb8373f9e69d6cf30a9554816e5710c`

## Limitations
The original approved source is a composite **JPEG poster**, not five original full-resolution raster animation strips. A 30px-high frame cannot show an entire tall poster scene without rescaling, so the fix uses close-up crops with black margins. This preserves actual proportions but necessarily hides parts of the original image.

No physical FM1 test has been performed. Hardware LCD refresh timing, power-on speaker-pop behavior and OTA installation remain to be validated on recoverable hardware.
