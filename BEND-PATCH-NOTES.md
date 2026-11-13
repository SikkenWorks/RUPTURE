# RUPTURE BEND patch notes

This source patch adds a new **BEND** play-parameter screen opened by **long-pressing FX**.

## Behavior
- **FX tap**: opens the regular FX page (unchanged).
- **FX long-press**: opens **BEND**.
- **K1**: momentary bend modulator.
  - Turning K1 raises the bend.
  - When you stop turning, the bend releases quickly.
- **K2**: bend target/range selector.
  - `1/4`, `1/2`, `FULL`, `1.5`, `DOUBLE`
- **Audio path**: the bend is applied to pitch **before** the FX engine.

## Visuals
- New BEND page UI in the native 240x240 1-bit RUPTURE UI.
- New pixel sprite animation: a mountain that rapidly shoots out of the ground while bending.
- Offline-generated source assets:
  - `vendor/etch-v1.88-source/firmware/fm1-native/rupture_bend_sprites.h`
  - `vendor/etch-v1.88-source/firmware/fm1-native/bend_frames/`

## Changed files
- `vendor/etch-v1.88-source/firmware/fm1-native/audio_core.h`
- `vendor/etch-v1.88-source/firmware/fm1-native/audio_core.c`
- `vendor/etch-v1.88-source/firmware/fm1-native/bringup.c`
- `vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c`
- `vendor/etch-v1.88-source/firmware/fm1-native/rupture_save_route.h`
- `vendor/etch-v1.88-source/firmware/fm1-native/rupture_host_export.c`
- `vendor/etch-v1.88-source/firmware/fm1-native/rupture_bend_sprites.h`
- `vendor/etch-v1.88-source/firmware/fm1-native/bend_frames/*`

## Validation
- Host syntax check performed with `gcc -fsyntax-only` on the modified native UI / audio / bringup sources.
- The RUPTURE V1.86 release has been cross-compiled with the JieLi/PI32V2 toolchain; see `RUPTURE-TEST-RESULTS-V1.86.md` for build, OTA and packaging audit.

## Follow-up: BEND UI pass

See `docs/BEND-UI-PASS.md` for the completed 5-position selector, current pitch display, 0–100% meter, IDLE/BENDING/RETURN indicator, native PNG previews and scope/validation details. This UI pass has no new hardware binary.

## V1.86.1 K3 response curves

K3 selects LINEAR / SOFT / HARD / S-CURVE / EXP. The selected response is applied to the temporary 0-100% K1 strength in the main control loop before conversion to oscillator pitch multiplier; all curves have identical endpoint pitch ranges. A mini-curve plot and three compact static-pixel knobs are displayed. The original mountain sprites, audio core, OTA loader, flash storage and Web MIDI Connection Fix remain intact.


## V1.87 Everest sprite refresh

- Swaps the BEND page mountain art for the approved `pixelberg_barst_uit_de_aarde.png` sheet, converted offline into eight native 224×30 monochrome frames.
- Keeps the V1.86.1 K3 curve system unchanged.
- Keeps the OTA connection fix and dual flash-ID compatibility unchanged.
- Touches only UI sprite assets/header, so the audio engine is not additionally loaded.

## V1.88: 5 curve-specific animations

Each K3 curve chooses its own eight-frame sprite set: LINEAR Everest (unchanged V1.87), SOFT monolith, HARD pirate flagship erupting from a meadow, S-CURVE ship close-up camera from hull to flag (no shake), EXP Kraken. Offline-generated 1-bit 224×30 frames in `vendor/etch-v1.88-source/firmware/fm1-native/bend_sprites_v188/` and packed read-only as a 5×8×30×28 uint8 array in `rupture_bend_sprites.h`. A four-phase native UI panel offset is applied when bending except S-CURVE; the real-time audio/FX and ISR code is untouched. The screen still respects existing audio-first LCD refresh throttling. Physical FM1 validation is required.

## V1.88.1 corrected source frames

The artwork for all five curve sets now comes from the exact user-approved poster image rather than the procedural approximation accidentally shipped as V1.88. Use `python3 tools/extract_approved_poster_frames_v1881.py` to regenerate the 40 packed sprites and the source crop archive. The poster is a composition, not 40 original source PNGs, so the native 224x30 artwork is resampled and converted to 1-bit. Every frame is tested for identical pixels between PNG and ROM header. K3 curve control, existing UI shake and full audio/OTA processing remain unchanged.

## V1.88.2 crop correction

All 40 frames now use 1:1 source crops with independent camera Y windows, black side margins, and no resize. The firmware sprite bank remains exactly 33,600 packed ROM bytes. No changes to DSP or updater.
