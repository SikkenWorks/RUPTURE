# RUPTURE V1.83 — Realtime Mono Headroom Test Report

## Source and build
- Source baseline: complete supplied RUPTURE V1.82 full-source project.
- Native build: JieLi pi32v2 with storage, updater, USB audio 48 kHz option and multi-project enabled.
- App: 348,000 bytes / 581,564-byte application budget, **233,564 bytes free**.
- Two consecutive clean native builds: **identical SHA-256** `ee57aa5da041f93696f85f5a3b18e1448014d4596d2dc46aa8e9abb5177d4438`.
- Firmware identity: `FM-1_968`; no app/flash offset or partition layout change.

## Implementation
1. RUPTURE hard-mono renderer now skips the dynamic 5–8-voice admission/headroom loop and variable-Q10 division per output sample. The mono gain divisor was always 4096 Q10, exactly 4 in the source signal domain. Upstream non-RUPTURE poly mode is unchanged.
2. Master-volume Q16 target, which is immutable during one DMA block, is computed once per block. Its existing per-sample gain ramp remains byte-identical.
3. No new audio buffers, ISR logging, FX instances, boot writes or storage changes.

## Regression evidence
- **Mono divider proof:** `tests/test_v183_mono_divider.c` checks each possible RUPTURE voice admission count (0, 1, 2) and every bounded integer sample from -524,288 through +524,288: exact equality to the V1.82 Q10 divide.
- **Rapid-note PCM comparison:** `tests/test_v183_audio_parity.py` compiles the *original* archived V1.82 audio core and new V1.83 core and replays rapid MIDI, SUB DRIFT, alternating held notes, sequencer gate/clock. Both have hash `4f603683ade7da59`, matching sample-for-sample.
- **Additional intensive host render:** `tests/bench_v183_headroom.c` ran with varying gain/drive, sustain, SUB DRIFT, sequencer, three reverb types, wave-preview and FX. Both cores produced hash `03f7ec8fa8b45876`, peak 18,189, in the 57,600-frame comparison.
- **Reverb stress:** `tests/test_v180_reverb_network.c` passed for all three independent engines, including 2,000,000 hot-switch samples and SIZE/DECAY tests. The `native_fx.c` source is byte-identical to V1.82.
- **Welcome/storage test:** `tests/test_v182_welcome_version.c` passed migration, subsequent reboot, CRC, two stored projects and simulated torn write. The storage/Welcome implementation is unchanged.
- **Native firmware package:** FWSC verifier passed; original 16 KiB protected bootheader and 6,930-byte trusted OTA-loader are byte-identical to V1.82. `ota.c` and `wave_store.c` are byte-identical to V1.82.
- **Installer:** standalone WebInstaller package, SHA-256 and bytes match the standalone FWSC; website build validation passed.

## Important limitations
- **No hardware FM1 test performed.** Real maximum DSP load, DMA LATE/GUARD increments, latency, USB-audio contention, and audible SUB+DRIFT crackles are not proven fixed.
- Host wall-time benchmark measurements were variable in the shared environment; no quantitative CPU improvement percentage is claimed. Eliminating the variable divide is a static code-path optimization, not a physical CPU-cycle measurement.
- The full historical test suite was not completed this iteration. The targeted tests above were run, but do not certify every firmware feature or every extreme combined-FX scenario.
- V1.83 preserves the V1.82 Welcome marker. Reinstalling stock V15 and the exact same RUPTURE build may remain indistinguishable from a reboot when V15 retains that journal.

## Hardware acceptance procedure
1. Back up projects and have a previously proven Pico/ROM rescue method ready before flashing.
2. Restart the FM1, enter GLO → AUDIO DIAGNOSTICS, record DSP MAX, MIN SLACK, DMA LATE and GUARD **before** testing.
3. Sustain and repeat fast notes with both SUB oscillators and DRIFT, then enable HALL, ROOM and SPRING in turn with all desired simultaneous FX. Change parameters while playing.
4. Record whether **DMA LATE and GUARD increase**, rather than interpreting cumulative nonzero totals by themselves. Repeat with USB audio connected if used live.
5. If either counter increases or the output crackles, the realtime performance target is **not yet achieved** and further code-level investigation is necessary.
