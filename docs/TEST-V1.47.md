# RUPTURE V1.47 — SAW SOUND SHAPING DSP and Firmware Test Results

## Reproduction and root cause
In V1.46 the four SOUND SHAPING controls were routed to existing CXS post-processing. The PUNCH effect was gated by `s->cxs.cycles < 8`, so it disappeared after eight carrier cycles. EDGE had its own SAW mapping, but WEIGHT/BITE could be less perceptible and still invoked the expensive 64-bit CXS shaper. The UI showed changes, but actual sustained SAW was not independently shaped by all four knobs.

## Fix
- The original CXS carrier and post-CXS shaping remain unchanged, preserving its PCM exactly.
- In SAW mode, WEIGHT reuses the existing lowpass state to add low body; EDGE remains the existing 0..100 tonal saturation control (50 neutral); BITE applies bounded odd-order soft harmonic shaping; PUNCH accentuates the residual saw edge continuously.
- The CXS-only post-shaper is skipped in fully SAW mode, removing its 64-bit operations from the sustained SAW path. During CXS ↔ SAW crossfade each carrier is shaped independently before the shared bounded mixer.
- No new voices, buffers, tables, storage fields, or boot/OTA modifications.

## Actual tests run
- Native FM1 pi32v2 crossbuild: PASS; app 241648 / 581564 bytes, +68 bytes versus V1.46.
- New V1.47 sustained SAW PCM test: PASS with UBSan. It ignores the first 60000 samples of each 150000-sample render and verifies distinct, meaningfully changed sustained energy at WEIGHT=90, EDGE=98, BITE=95, PUNCH=95. PUNCH remains audible in late audio.
- New neutral-parity test: PASS, 200000 samples equal to V1.46 neutral CXS post-shaper output.
- New alternating-parameter overdrive test: PASS, 600000 samples bounded with UBSan.
- Existing V1.46 SAW PLAY controls test: PASS with UBSan.
- Existing V1.45 oscillator/suboscillator/screensaver test: PASS with UBSan.
- Existing V1.45 PJT project encode/decode compatibility test: PASS with RUPTURE_BUILD and UBSan.
- FM1 protected boot region and OTA loader: verified in tools/build_release.py; firmware/site hash parity verified by site release tests.

## Not claimed
- No test has been performed on a physical M-VAVE FM1 for V1.47. Listening quality, tactile encoder behaviour and worst-case DMA margin remain hardware checks. The entire historical regression suite was not run in this release.
