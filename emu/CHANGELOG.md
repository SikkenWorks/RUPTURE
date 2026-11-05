## V2.4 - Two editions (RUPTURE bundled)

- Split V2.3 into two independently deployable GitHub Pages PWAs.
- Added firmware-edition policy, dedicated tests, scoped offline service-worker cache and updated PWA metadata.
- Bundled exactly the supplied RUPTURE.fwsc, pinned its SHA-256, and blocked firmware import / switching paths including backup files from other firmware.
- Kept Joakim Eriksson's emulator MIT license and attribution.

# Changelog

## V2.3 — 2026-11-05 — Audio Clock & Input Response

- **Fixed a real timing mismatch** when a browser opens its AudioContext at a rate other than 44,100 Hz despite the request. The FM-1's emulated 44.1 kHz PCM is converted **continuously** to the actual output rate in the AudioWorklet, rather than being played as if it had that rate.
- Added a fractional-phase resampler with samples retained across AudioWorklet callbacks; avoids discontinuities at each 128-frame block. At 44.1 kHz the source audio is passed through unchanged.
- Added an AudioContext fallback for mobile browsers that reject an explicit 44.1 kHz request, and disclosed source/output rates and conversion in Diagnostics.
- Added **Response** in the toolbar: Fast (256 source frames), Balanced (512), Stable (1024, the unchanged default). It adjusts the emulator's look-ahead buffer after a safe firmware restart; smaller queues can improve input response but risk dropouts on slower devices.
- Added tests for 44.1 kHz passthrough, 48 kHz pitch/time preservation, callback continuity, underrun silence, 32 kHz source rate conversion, rejected-context fallback, and response selection.
- Retained the original emulator, `.fwsc` loading, Web MIDI, saves, touch/keyboard, editable shortcuts, Hardware Accuracy mode, diagnostics, offline WAV comparison, PWA and upstream credits.

**Validation boundary:** Tests use synthetic audio in a simulated AudioWorklet. No real physical FM-1 recordings, browser device measurements or fresh Rust/WASM compilation were available for this build. Conversion prevents a known browser-rate mismatch; it does **not** establish chip-cycle accuracy or solve the remaining unverified peripheral timing differences.

## V2.2 — 2026-11-05 — Hardware Calibration Lab

- Added browser-side **Physical FM-1 Audio Comparison** to Diagnostics (no new obstructing toolbar).
- Import two local 16-bit PCM WAV captures, mono or stereo, up to 60 seconds at 8–96 kHz.
- Match timing using RMS-onset envelopes and waveform correlation. Report lag, signed polarity, correlation, level mismatch and residual after gain matching.
- Export only comparison measurements as JSON, not raw audio or MIDI.
- Reject mismatched sample rates, silence, malformed WAVs and unsupported formats with actionable errors.
- Added five independent WAV parsing/comparison tests, including known delay, gain, inverted polarity, mismatch and bad files.
- PWA cache, browser deployment build and GitHub Actions tests updated.
- Original Joakim Eriksson credits and MIT license remain; the emulation engine and flash behavior are unchanged.

**Limitations:** this is a calibration **measurement workflow**, not automatic peripheral timing tuning. No Rust/WASM rebuild or physical FM-1 measurements were possible here. The GitHub Actions build must complete before the full engine is available.

## V2.1 — 2026-11-04 — Accuracy Lab

- Optional ten-second diagnostics in the existing compact toolbar, with private, locally exported JSON results.
- AudioWorklet instrumentation is opt-in: p50/p95/p99 callback duration, peak/RMS, clipping samples, underrun counts and observed LCD/LED changes, without recording raw audio/MIDI data.
- Diagnostics automatically stop when the emulated firmware or CPU clock is restarted.
- JSON reports identify firmware SHA-256, emulation mode and MHz, and explicitly say they do **not** prove hardware accuracy.
- Added a standalone 16-bit PCM WAV comparison program that aligns captures and reports latency, correlation, polarity, RMS difference and residual error.
- Added MIDI-only benchmark script conversion to standard MIDI Type 0; supplied one deterministic note sequence in both formats.
- The offline WebAssembly benchmark can run Hardware Accuracy (`FM1_STRICT=1`) and export WAV for physical-versus-emulator comparisons.
- Updated PWA service worker version, GitHub Pages build, and JavaScript/Python regression tests.
- Retained Joakim Eriksson credits and the original MIT license, all V2.0 features, saves, MIDI, shortcuts and .fwsc imports.

### Verification still required

- Fresh Rust/WebAssembly build and boot with actual .fwsc firmware (GitHub Actions performs the build; it was not available locally).
- Real FM1 analog audio, LCD, control and clock comparison; none was fabricated or simulated as a physical measurement.


## V2.0 — 2026-11-04

- Event-driven, dirty-state browser panel renderer: skips redraws while the FM-1 is idle.
- Cached `ImageData` and automatic refresh when WebAssembly memory grows.
- Worklet sends only changed LCD, LED and pad data; browser merges unpainted frames.
- Startup JIT compilation capped at six regions/quantum and two while audio is running.
- `fm1_strict` Rust export exposes existing native hardware-accurate timing model to the WebAssembly emulator, including 8-tick instruction cycle costs, misalignment, XIP cache, timed LCD; interpreter only in this mode.
- Mode selector, first-switch 240 MHz reference, safe restart and persisted Performance clock.
- Service worker caches local WebAssembly engine when included in built release; offline cache version updated.
- Added regression tests for paint scheduling, strict-mode boot, timing budgets and reduced frame traffic.
- Kept original Joakim Eriksson credit link, original MIT license, `.fwsc` import, saves, MIDI, shortcut editor and original firmware behavior.

### Unverified

- No local Rust toolchain available in this environment: fresh V2 WASM **was not compiled here**. GitHub Actions build is included.
- No end-to-end custom `.fwsc` boot or physical FM-1 audio/timing comparison performed in this environment.
- No claim of cycle-perfect simulation or a quantified speed gain.
