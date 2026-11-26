# RUPTURE V2.0.59 — targeted rebuild validation

## Status

V2.0.59 was re-created directly from the full V2.0.58 source because a prior claimed V2.0.59 release did not exist in the active file runtime. This is a **new independently rebuilt release**, not a recovered byte-identical copy of the missing version. Both images are **hardware-unverified**. CORE2 is experimental and should be flashed only with proven recovery hardware.

## Changes

- KEY PRESSURE reads a wider ADC delta and limits the per-foreground-step rise/fall to avoid sudden 99–100% jumps. The digital key matrix owns all Note ON/OFF events.
- MOSFET MIX, DISTORTION MIX, REVERB MIX and WAH MIX now modulate actual wet/dry controls. MOSFET uses a runtime-only encoding (p3=1 dry through p3=32767 wet), with the historical stored value p3=0 continuing to mean original 100% wet.
- The FX edit screens show the active modulation's LIVE MIX and BASE/MIN range for these four effect types. Unmodified CXS and original effect formulas are preserved; adding MOSFET MIX intentionally changes its output only when that modulation is enabled.
- All original firmware update protections and CORE2 fallback stay in place.

## Performed checks

- Native JieLi pi32v2 builds succeeded: STABLE 522268-byte app and CORE2-FX 529244-byte app. Neither exceeds the 581564-byte app allocation.
- Both FWSC containers produced and verified, with identical unchanged 16 KiB protected header and OTA loader.
- The installer embeds and SHA-256 verifies both exact FWSCs; STABLE remains the default.
- Host pressure+FX tests passed: gradual pressure increments, reverb/distortion/wah/MOSFET MIX control, original DRIVE unchanged.
- Native MOSFET PCM test passed: dry=12000, middle=12036, full wet=12072, legacy wet=12072 (one deterministic input and preset).
- Legacy host KEY PRESSURE and separate CXS/GROWL profile tests passed.
- Host CORE2 PCM parity passed: 100 FX pairs + six heavy chains (81408 frames), 384000 frames with injected CORE2 failure scenarios matched exactly.
- CXS DSP source byte-identical to V2.0.58.

## Outstanding

- The entire historical regression suite has NOT been repeated for this rebuilt revision.
- The pressure sensor curve, timing, DMA LATES, note-off behavior under extended real-world use, and true parallel CORE2 execution still require prolonged testing on physical FM1 hardware.
- Release remains a test candidate. Preserve a confirmed working previous FWSC and an independent recovery route before flashing CORE2.
