# RUPTURE V1.79 — SUB DRIFT / DSP Deadline / DMA Guard Audit

Date: 2026-11-04. Baseline: RUPTURE V1.78 full native source project. **Software-tested candidate; NOT an FM1 hardware-certified live/studio release.**

## Reported real-device symptoms

FM1 user reports audible crackling when enabling a SUB oscillator and turning DRIFT; device AUDIO DIAGNOSTICS displayed DSP MAX = **89%**, DMA LATE = **28**, GUARD = **23**. These are observations on the user's physical device, NOT values produced by our host tools. `DSP MAX` is historical (peak since boot); DMA LATE/GUARD are cumulative, not 28 and 23 glitches per second. Rising LATE/GUARD while playing means real DMA deadlines or ownership checks failed. Crackling may have multiple causes; the observed figures make a realtime overrun a credible explanation, not yet hardware-confirmed root-cause proof.

## Confirmed avoidable per-sample costs in V1.78

1. `rupture_subosc.h`: 64-bit multiplication in `ru_sub_inc` on EVERY sample of EACH enabled SUB, including during continuous DRIFT control interpolation. Replaced with two 32-bit partial-product multiplies and guarded additions. This gives **exactly the same saturated 32-bit Q16 note increment for every in-range detune/drift/octave value**, with a defensive full-precision fallback only for invalid/non-musical factor values. No quantization of pitch to blocks, no reduced drift rate, no changed waveform.
2. `native_fx.c`: 64-bit multiplications/divisions in both L/R HALL, ROOM, SPRING dry/wet mixes, plus 11 other per-sample wet/dry FX interpolation sites. For Q15 audio signals (the valid normal range), use mathematically EXACT 32-bit integer arithmetic; for out-of-range, preserve the V1.78 expression. No changes to reverb feedback networks, delays, impulse responses or samples.
3. `bringup.c`, `rupture_lcd_budget.h`: under measured recent audio wall-time pressure, successive SUB/FX knob turns forced an LCD redraw at every 25ms service tick, competing with audio/USB DMA. Coalesce *knob-only* paint requests to 50ms for moderate pressure, 75ms for high pressure, 100ms for repeated misses. The control value and corresponding audio parameter are still serviced at the original 25ms tick. Note inputs, page changes, transport, notices and fault displays still trigger immediate rendering. Noncritical sprite animation is capped to 20fps under recent high audio wall-time pressure; visual state continues to advance correctly. The UI may briefly refresh less smoothly; the PCM path is untouched.

## Test results

| Verification | Result |
|---|---|
| Native JieLi pi32v2 crossbuild, storage/updater/USB 44.1/48 configurations | PASS (full native build with USB 48 kHz option, 44.1 also compiled into target) |
| App image audit | PASS, **347,352 bytes** of 581,564-byte application area; **234,212 bytes** free |
| Two native builds | PASS, byte-identical SHA-256 `c9a7ff1644f71e75bcefec26b99443548f06a41dbc7c36f7c2f84fe244af57cb` |
| SUB Q16 increment reference, varied octaves, detunes, and drift | PASS, **700,000+ exact matches** |
| Two simultaneous SUBs with repeatedly moving DRIFT, classic/CXS/square/saw | PASS, same PCM hash as original V1.78: `ab2ab9cc` |
| Whole-FX PCM stress: all enabled FX, continuously changing parameters, all three reverbs | PASS, **780,000 bytes byte-identical** to V1.78, SHA-256 `93b98d02f77d868766cec5630a7b624133feebb7de0e2f37b55f6b54f30cea23` |
| Three genuinely separate Hall/Room/Spring reverb outputs and project roundtrip | PASS, same V1.78 unique PCM hashes, wet-only 100% |
| Dual SUB + global effects peak stress test | PASS, no full-scale clipping, peak 26,346 |
| Sequencer nearest-step quantization, countdown transition, LED logic | PASS, selected regressions |
| New LCD/knob pressure rate and urgent UI behavior | PASS |
| Webinstaller / embedded firmware SHA / release identity | PASS |
| Protected first 16 KiB of flash and trusted OTA-loader payload | PASS, **byte-identical to supplied V1.71 reference** |
| `ota.c`, `audio_core.c`, `native_project_codec.c`, hardware ISR assembly compared to V1.78 | PASS, all **byte-identical** |
| Full historical regression suite | NOT PASS: an old source-contract check `test_v176_source_contract.py` expects a V1.76 hash of `native_project_codec.c`, which already changed in the V1.78 baseline; an earlier V1.27 UI-pixel assertion also remains unresolved. Independent relevant tests above passed. |
| Actual pi32v2 realtime cycle profile, all FX/max controls, both SUBs, screen redraws | **NOT PERFORMED** (no physical FM1 access) |
| Physical long-run DMA LATE / GUARD / underrun count after update | **NOT PERFORMED** |
| Hearing crackle-free audio on the user's unit | **NOT YET VERIFIED** |

## Required physical acceptance before calling this live/studio stable

1. Save projects; use the verified Pico recovery route and avoid flashing if not comfortable with experimental candidate builds.
2. On FM1, cold boot and note baseline AUDIO DIAGNOSTICS `DSP MAX`, `WALL MAX`, `RENDER`, `PUBLISH`, `MIN SLACK`, `LATE`, `GUARD` before the stress test.
3. Play sustained notes using both SUBs, rapid DRIFT sweeps, high-detune/high-octave, SAW and CXS; set both global/sub FX, distortion, compression, EQ2, trem, phaser, flanger, tape, reverse, vibrato, rotary, wah, reverb to strongest valid settings. Keep the full audio quality.
4. Test the same with sequencer recording, metronome, heavy USB MIDI and USB audio, fast screen changes and encoder movements. Test long sessions with full output and normal gain settings.
5. **Pass:** no audible clicks or pops from processing, **no increase** in LATE/GUARD from their starting values, positive MIN SLACK with sufficient margin throughout the test. An old nonzero counter alone is not proof of ongoing failure; a fresh increase is.
6. If LATE or GUARD increases, STOP relying on the test build for live use; send the before/after diagnostic readings, active patch/effects, sampling rate and whether USB audio is attached. Further target-specific optimization and hardware timing changes will be needed. Absolute all-load headroom cannot be proven from cross-compilation or PCM hashes.

## OTA/recovery boundary

New `.fwsc` was generated with the same exact protected 16 KiB header and trusted OTA loader as the supplied V1.71 package. Cross-firmware compatibility and restoring the FM1 were not exercised on hardware. Device identity `FM-1_968`, storage project codec and OTA state machine are unchanged. A valid `.fwsc` does not prove a safe real-device OTA. Keep verified physical recovery.
