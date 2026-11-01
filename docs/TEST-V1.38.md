# RUPTURE V1.38 — Full filter & mono DMA load audit

## Reported physical issue

The author reports that all RUPTURE synth filters crackle, and the LADDER filter still produces `AUDIO STOP / DMA TIMING FAULT` on a physical M-VAVE FM1 after V1.37. The symptom is real user feedback but **cannot be reproduced or timed on the physical FM1 from this environment**.

## Confirmed source-level problems

1. Four filter modes were doing repeated coefficient calculations inside the 44.1 kHz / sample / voice filter path, including at steady settings. V1.37 improved the ladder multiplication but did not remove the repeated resonance coefficient mapping.
2. RUPTURE's actual `render_internal()` forcibly limits itself to **one active synth voice** regardless of the nominal `voice_limit`. The original `native_budget_late()` tried reducing `voice_limit` (8→6→4→3→2) as the first overload response. That **cannot reduce any filter CPU time in RUPTURE mono**. Consecutive missed DMA deadlines eventually still call `native_stop()`.
3. The existing display / scope pressure feedback can defer nonessential visual work, but the prior mono-budget fallback did not request the maximum relief immediately on the first confirmed late DMA half.
4. The DMA pre-write ownership guard remains an essential safety check. It cannot be suppressed or worked around to make an apparent success.

## Changes

- Added prepared coefficients for four-stage LADDER and two-iteration Chamberlin LOWPASS / BANDPASS / HIGHPASS. The hot loop reuses coefficients while cutoff/resonance are unchanged; they are recomputed when the parameter changes. GROUPS/per-voice differences receive their own freshly prepared coefficients when necessary. No change to filter curves, feedback, 24 dB pole count, saturation, Q15 rounding, ADSR, voice count, or project settings.
- Coefficient caching is **render-stack-local** (no heap, no extra DSP POOL, no new persistent RAM). Main-loop/UI never accesses filter state.
- On a real late DMA half, RUPTURE mono immediately sets maximum existing UI/scope pressure, instead of executing an ineffective polyphonic voice-budget reduction. Repeated late halves continue to stop the audio for safety; DMA preflight, UAC and recovery logic remain intact.
- The non-RUPTURE ETCH firmware path and exact protected bootheader/OTA loader are untouched.

## Software checks

- JieLi pi32v2 build: pass, app fits within 581,564-byte application limit.
- Previous V1.37 LADDER direct kernel test: **972,000 exact output/state comparisons** pass.
- New mixed-mode filter test: **2,000,000 pseudo-random and changing-control samples** comparing the original mathematical formulas and the new prepared kernels across four voice states, plus **500,000 stable resonant ladder samples**. Every filter output and internal state matches exactly. Undefined-behavior sanitizer enabled for the new test.
- Existing historic regressions and installer/site checks: see `regress138.log` / `regress138.exit` when included in the release.
- Firmware package CRC, product identity, original bootheader (first 16 KiB) and original OTA-loader byte match: run during packaging.

## Remaining physical gate

This is a **verified source-level load reduction, not proof of a fixed real-hardware crash**. Host test runtimes are not FM1 cycle timings. Measure GLO → AUDIO DIAGNOSTICS `DSP MAX`, `WALL MAX`, `PUBLISH`, `MIN SLACK`, `LATE`, `GUARD`, and `SCOPE SKIP` on the FM1 at 44.1 kHz. Compare FILTER OFF, LOWPASS, BANDPASS, HIGHPASS and LADDER using the same held note / CXS and FX settings. Test sustained high resonance, fast sweeps, maximum drive, USB connected/disconnected, sequencer playing and dense screen activity. If `LATE` or `GUARD` continues to rise, or if there is still clipping/crackle, do not claim the issue resolved. The original DMA fault response remains intentionally active. Save projects and keep a known-safe firmware/recovery route before flashing.

### Final offline results

- Historical `tests/run_tests.sh`: complete, exit code **0**. The V1.38 new test has also been run independently with `-fsanitize=undefined` and no errors.
- Reproducible JieLi build: two independent native build runs produced **byte-identical** `app.bin` binaries.
- Application binary **228,796 bytes** / **581,564-byte** app slot, **352,768 bytes** of app flash headroom. This number is **flash space, NOT a realtime CPU headroom measurement**.
- Firmware FWSC package **610,086 bytes**, identity `FM-1_968`, unchanged trusted OTA loader and protected 16 KiB boot header.
- The site `index.html` and standalone `WebInstaller.html` embed the same package bytes and SHA-256.
