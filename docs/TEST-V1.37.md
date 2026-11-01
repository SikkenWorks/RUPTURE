# RUPTURE V1.37 — Ladder DSP and DMA fault audit

## Reported physical symptom

On an FM1 running V1.36, choosing the 24 dB/oct LADDER filter can produce crackling followed by a screen that says SAFE MODE. This is a **user-reported hardware symptom**, not independently reproduced in this environment.

## Verified source-level finding

The native audio code runs in the ALNK0 DMA interrupt. The V1.36 ladder performed five 64-bit signed multiplies for each voice and sample (one feedback multiply plus four stage integrates). RUPTURE's audio guard rejects writes when a DMA half has expired. After enough consecutive missed deadlines at the minimum voice budget it calls `native_stop()` and sets `native_tripped`. The original UI mapped either `native_safe` **or** `native_tripped` to the SAFE MODE screen, so a realtime audio deadline stop could look like a recovery failure. This source-level path is real; whether it exactly explains the user's physical occurrence requires a hardware measurement of `LATE`, `GUARD` and render/DMA wall time.

## Changes

1. Replace all five ladder 64-bit multiplies per sample/voice with mathematically equivalent, bounded 32-bit Q15 operations. The coefficient and resonance mapping, four-pole cascade, saturation and audible gain remain otherwise unchanged.
2. Clamp the ladder's input to legal Q15 bounds (does not affect ordinary audio samples).
3. Keep all native DMA ownership checks, automatic voice-budget reductions, and protective audio-stop behavior enabled. The fix does not conceal audio overruns.
4. Distinguish `AUDIO STOP / DMA TIMING FAULT` (which HOME can restart) from a real boot/recovery SAFE MODE in the 1-bit native UI. This is a diagnostic/UI correction, not a recovery-policy change.
5. No changes to project format PJT34, EQ2, USB MIDI, OTA loader, bootloader, flash map or stored presets.

## Evidence

| Check | Result |
|---|---|
| Sanitized host ladder comparison | **PASS**, 972,000 exact output/state comparisons against original V1.36 code |
| Extreme cutoff and resonance sweep | **PASS**, 8 cutoff settings × 7 resonance settings, 12,000 samples per pair |
| Rapid parameter automation | **PASS**, 300,000 extra samples |
| Bounds with malformed upstream full-width input | **PASS**, 10,000 randomized-sign extremes limited safely |
| Native JieLi pi32v2 crossbuild | **PASS**, final build `.fwsc` app layout within limit |
| User data, boot header and OTA consistency | **PASS**, unchanged protected first 16 KiB and exact trusted OTA body |
| Real FM1 crackle, audio deadline, SAFE MODE | **NOT TESTED in this environment** |

## What remains to test on an actual FM1

Back up projects and keep your tested recovery route. After installing, use a simple patch, then test LADDER at low/mid/high cutoff, maximum resonance, maximum drive, both subs, effects and rapid knob sweeps. On GLO → AUDIO DIAGNOSTICS, watch LATE, GUARD and audio-render wall-time. A successful crossbuild and sample-accurate host comparison **do not prove** that the symptom is completely fixed on real hardware. Stop if output repeatedly crackles or `AUDIO STOP` appears and collect the diagnostics; do not disable the safety watchdog or repeatedly flash blind.
