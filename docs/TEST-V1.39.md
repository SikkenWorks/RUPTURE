# RUPTURE V1.39 — Dry audio / DMA regression audit

## Physical symptom and scope

User reports crackles and repeated DMA deadline audio stops on V1.38, including with all synth filters and user-facing effects disabled. No physical device is accessible in this build environment. This release fixes a confirmed, directly exercised CPU-heavy code path, but a real FM1 audio soak and deadline measurements remain essential before asserting the symptom has disappeared on hardware.

## Root cause found in the source

`native_fx.c` prevented the existing complete dry-FX bypass whenever `s->eq2_enabled` was true. New PJT34 projects enable the four-band EQ2 by default. Critically, all four bands at exactly 0 dB still ran through `rupture_eq2_process` and the wider effects chain. Every EQ2 biquad used a Q26/64-bit filter multiply and per-sample parameter smoothing. Thus FILTER OFF and FX mixes at zero did **not** imply an inexpensive dry audio path. The old V1.37/V1.38 work optimized only the synth filters and mono overload action; it did not remove this hidden always-on EQ2 work.

## Native source fixes

1. `rupture_eq2_is_unity()` tests *both* the currently applied and targeted coefficients, as well as actual and target gain compensation. This avoids treating a still-smoothing active EQ as a bypass.
2. Global dry FX fast path now works when EQ2 is enabled but neutral, and also when explicitly bypassed. The neutral path skips actual four-band EQ DSP and the complete inactive FX chain, while maintaining only two Q15 input/output history samples per band so activation does not begin from stale history.
3. Active EQ2 processing skips the five Q26 filter products in any band whose currently applied coefficients are neutral. Still updates each band's histories, allowing active non-neutral neighbors and later parameter automation.
4. Neutral pitch-bend avoids a redundant 64-bit product per PCM sample, and stable FILTER OFF avoids per-sample transition calculations. Nonneutral modulation, filter mode changes, Q values and the original four filter algorithms remain supported.
5. Existing DMA half-ownership preflight, 4096 TIMER4-tick publish reserve, repeated-deadline shutdown, bootheader, trusted OTA loader and PJT34 storage format remain unchanged. We did **not** disable crash detection or silently drop audio samples.

## Verified tests

- Native pi32v2 crossbuild (`--storage --updater --usb-audio-48k`): PASS, 229068-byte app, 352496-byte space headroom in the application flash limit.
- Native app entry and linker integrity: PASS.
- Vendor HAL/boot core audit: 19 unchanged hashes, PASS (not a claim of USB/OTA parity).
- New 240,000-sample dry audio regression: EQ2 ON/neutral renders exact input, no active EQ processing, internal history remains valid; PASS.
- Turning on an EQ band changes output; returning all bands to 0 dB eventually returns to exact fast dry; bypassed EQ also runs dry: PASS.
- New test with undefined behavior sanitizer (`-fsanitize=undefined -fno-sanitize-recover=undefined`): PASS.
- Existing V1.34 EQ magnitude/Q width/neutral channel EQ tests: PASS.
- Existing 972,000-sample LADDER bit-exact test: PASS.
- Existing 2,000,000 mixed-filter + 500,000 fixed-LADDER samples: PASS.
- Browser/standalone installer SHA-256, exact byte parity, metadata, documentation: PASS.
- V1.38 -> V1.39 first 16 KiB protected flash-header: byte-identical; `ota.bin` loader: byte-identical.
- Wider historical `tests/run_tests.sh`: PASS through V1.23 with no observed failures, then exceeded execution limit; **not a claim of a fully completed historical suite**.

## CPU benchmark (host only)

Same source-level workload, `gcc -O2`, 1,000,000 neutral-EQ2 dry stereo samples:

| Firmware code | Host CPU time | PCM checksum |
|---|---:|---|
| V1.38 | 76.638 ms | 2702728928 |
| V1.39 with retained EQ history | 30.277 ms | 2702728928 |

This is a roughly 2.53x speedup in **host** dry-FX processing, not a pi32v2 DMA timing number. Other audio stages, interrupt nesting, SPI/UAC work and LCD scheduling are not included in this benchmark.

## Remaining physical gate

On a recoverable FM1, make a project backup, install the *exact* packaged V1.39 FWSC, and compare long sustained notes on FILTER OFF / all FX off with the former offending patch. Then test Channel EQ 2.0 with one and multiple active bands, LADDER at high resonance, sequencer running, MIDI and USB attached/unattached. Under GLO > AUDIO DIAGNOSTICS record the LATE, GUARD, MIN SLACK and SCOPE SKIP counters. A lasting positive MIN SLACK and no increases in LATE/GUARD are necessary before treating the hardware report as resolved. Any repeated DMA stop after this version calls for a real timing trace and potential further scheduling work.
