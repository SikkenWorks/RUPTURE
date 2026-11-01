# RUPTURE V1.4 test results

- Native JieLi pi32v2 build: **PASS**, app 174220 bytes, SHA-256 `ec9da14c18f7502b95559d511e8c298aaf968fbc585839d7f301f67fedab94e0`.
- Native second build: **byte-identical**.
- Complete host test suite (including CXS, MIDI, FX, sequence, source UI): **PASS**.
- PJT32 Sound Shape roundtrip, V1.3 default loading and non-RUPTURE marker rejection: **PASS**.
- 28 1-bit PNGs: exported from the exact native C UI renderer, 240×240 pixels.
- `.fwsc` identity FM-1_968 / app CRC and reference boot/loader parity: **PASS**.
- Firmware package 610086 bytes, SHA-256 `194db5a2460f5a8d5e81547092a1c95c50792a4010478b038502e3e9a1bcbb43`.
- Installer embed and SHA-256: **exact match**.
- FM1 physical flash/boot/audio/MIDI test: **not performed**. Tests prove build and simulated storage serialization, not real flash compatibility.


## V1.5

- Native JieLi pi32v2 build: PASS; repeat SHA-256 identical.
- Portable legacy DSP/UI/MIDI/project tests: PASS.
- 12-slot NOR flash emulator: PASS for all project slots and CXS shape memory, legacy four-slot migration, failed mid-write fallback and project deletion.
- Protected 16-KiB boot header, original OTA loader, app CRC, WebInstaller package SHA-256: verified after packaging.
- Physical FM1 installation, storage durability and cross-firmware downgrade: NOT TESTED.

## V1.6

- JieLi pi32v2 crossbuild passed; app 174768 bytes, SHA-256 38ce523db8d305f8aabca712342cac626bc66cf5a8e44e0fa98aeae5dacfba9c.
- All RUPTURE portable regressions including 12-slot flash model passed.
- Note Memory priority and glide helper tests passed; native audio caller compiles.
- Firmware UFW and CRC verified; protected header and OTA retained.
- FM1 physical play/USB/TRS tests and power-cycle persistence are not verified.

## V1.7

- Crosscompile and binary audit: PASS.
- CXS/MIDI/FX/flash bank regressions: PASS.
- SAVE/REC bit routing: native source verified; not tested on physical panel.
- SEQ velocity and auto-save guard tests: PASS.
- .fwsc integrity and OTA bootheader parity: PASS; SHA-256 `3cd287d0d4c2e215c11f3f3de09afd260768161287cd753640a38825b97def2c`.
- Native UI screenshot exports: 28 strict 1-bit images.
- Physical MIDI, flash, charger detection, long-term autosave wear and reboot tests: NOT RUN.

## V1.7

- Crosscompile and binary audit: PASS.
- CXS/MIDI/FX/flash bank regressions: PASS.
- SAVE/REC bit routing: native source verified; not tested on physical panel.
- SEQ velocity and auto-save guard tests: PASS.
- .fwsc integrity and OTA bootheader parity: PASS; SHA-256 `0ba7e24fde8d123ee5e8c060fb865b3011f3b8d04537b315af3e45423ccc191a`.
- Native UI screenshot exports: 28 strict 1-bit images.
- Physical MIDI, flash, charger detection, long-term autosave wear and reboot tests: NOT RUN.

## RUPTURE V1.8
- Native JieLi app build: PASS; 177808 bytes; SHA256 9ad9f7414b08947759fbd6ced6de62374ef251044b7e562dcd22d9c91ba9843a
- Name serialization through unmodified PJT32 codec: PASS, eight slots.
- 16-key white-step mapping, 4/8/16/32/128 window policy: PASS.
- RUPTURE native 1-bit screen renders: 29 exported at 240x240.
- .fwsc verifier: PASS; SHA256 92770427da9437c4804d3a4385bd9bc8e071e985430548175b94f66fbd416625.
- Protected boot and trusted OTA loader: byte-for-byte match V1.7.
- Hardware switch/button and LED playback validation: NOT RUN.

## RUPTURE V1.8
- Native JieLi app build: PASS; 177808 bytes; SHA256 9ad9f7414b08947759fbd6ced6de62374ef251044b7e562dcd22d9c91ba9843a
- Name serialization through unmodified PJT32 codec: PASS, eight slots.
- 16-key white-step mapping, 4/8/16/32/128 window policy: PASS.
- RUPTURE native 1-bit screen renders: 29 exported at 240x240.
- .fwsc verifier: PASS; SHA256 92770427da9437c4804d3a4385bd9bc8e071e985430548175b94f66fbd416625.
- Protected boot and trusted OTA loader: byte-for-byte match V1.7.
- Hardware switch/button and LED playback validation: NOT RUN.

## RUPTURE V1.9
- Native JieLi app compilation PASS: 179908 bytes
- Dual CXS/classic sub tests PASS: octave, detune, drift, fade, OFF parity, gain headroom
- PNG renders PASS: 31 monochrome 240x240 exports from native C renderer
- .fwsc validation PASS: identity FM-1_968, SHA-256 103a6bafa80c364f484c382875b846320b8610dc2cc680897d023775ba4b98b1
- Existing 16 KiB bootheader and trusted loader byte-for-byte unchanged from V1.8
- Physical audio/encoder/OTA test NOT RUN

## RUPTURE V1.10
- FM1 app crosscompile PASS: 180336 bytes
- SUB 1/2 label regression tests PASS: TYPE/OCTAVE/DETUNE/DRIFT
- Both-off original mix exact PASS
- Main level no attenuation when subs active; additive mix + peak guard PASS
- SUB 1/2 CLASSIC harmonic output and detune energy PASS
- Existing regression suite PASS
- .fwsc package valid: 610086 bytes, SHA-256 20fb9dfcfd052df0eed40b6f08c8d01637602dea5105fb9d46d2798d8f4ac353
- Protected bootheader and trusted OTA loader unchanged compared with V1.9
- Physical FM1 test NOT RUN

## RUPTURE V1.11
- Native JieLi pi32v2 repeat build: PASS, byte-identical, app 181028 B, SHA-256 `88be6009ae634c00a65ea48154cbfd0131710e0100a0445e83e0f9f898ef4349`.
- RUPTURE SUB 1/2 PJT32 encode/decode: PASS, two distinct presets, bit-6 sub marker + bit-7 shape marker coexistence.
- Legacy preset missing sub tag: PASS (both SUBs OFF).
- Invalid sub marker and foreign patch ownership: rejected before commit (PASS).
- Existing CXS, FX, sequence, twelve-slot storage, note memory and additive SUB host regressions: PASS.
- FWSC validation: PASS, SHA-256 `737561aa57487fe4ae54bc21a5c4ba7814261e3b98621ef06cff38513ba1c992`.
- V1.10 protected 16 KiB boot header and OTA loader: byte-for-byte unchanged.
- Hardware save/reboot, OTA transition and physical audio: NOT TESTED.


## V1.12 validation
- Native pi32v2 crossbuild: PASS, 181436-byte app.bin; `_start=0x02000120`; headroom 400128 bytes.
- Full PJT32 tagged NOTE MEMORY + Master Volume encode/decode: PASS, including 0%/200%, all priority modes, 0–400 ms glide, bend values, old-record defaults and malformed marker.
- Previous regression suite: all tests through V1.11 emitted PASS; full-suite invocation timed out before the new V1.12 case ran; V1.12 case separately passed.
- Screens: unchanged native C renderer, 31 exports checked; no hand-drawn mockups.
- Flash package, bootheader, OTA loader: validated against V1.11 and V1.88 reference.
- Physical FM1 boot/save/reboot, cross-firmware restoration: NOT TESTED.


## V1.13 validation
- Full portable host regression suite: PASS, including 12-project legacy migration, Note Memory, SUB storage, SOUND SHAPE, USB/TRS parser, and CXS DSP.
- New NOR model rejects any SPI page program longer than 256 bytes or crossing a 256-byte boundary; twelve project saves, volume record, torn write fallback and 28-record A/B compaction: PASS.
- PJ52 manifest write corrected to two page-safe SPI NOR operations. Previous single 296-byte write was not supported by the actual FM1 HAL.
- Native JieLi pi32v2 app validation PASS; app 182824 bytes; SHA-256 `2263836f1a52d2b98a8dd55c448bbb5c08807c6f16004b3e801918fc549bbc87`.
- FWSC UFW verification PASS; package 610086 bytes; SHA-256 `1e0155ce16c60aa828844eb6afaa1732407975f46c99de0831fe61fbd416b11a`.
- Protected bootheader and OTA loader: byte-identical to archived reference.
- Physical FM1 tests: NOT PERFORMED; no claim of real-world save or charging reliability.


## V1.14 FX transition and RAM validation
- Platform pi32v2 native compile: PASS; app 182,808 bytes, SHA-256 `1e8642816d43b354e0afeabab8a94918d4f66bc3692cf0d0d1cee8345667433b`.
- Full portable regression suite: PASS; new bypass fade/dry-parity test: PASS.
- FX shared state: +8 bytes in .pool; no new buffers. .pool unallocated: 9,028 B, .noinit unallocated: 3,500 B.
- Firmware UFW/CRC and identity: PASS; SHA-256 `8a73923b9139fca500eebb8c605d3eccbcc365d37894af1ed8b1daabfe1e6bfd`.
- Original bootheader and trusted OTA loader: byte-identical to V1.88 archive.
- FM1 CPU profiling, physical switches, audio and OTA: NOT PERFORMED.

## V1.15 FX tests
- Native pi32v2 app 185032 B, SHA-256 `ffc41695ff7fa95f5cda855a56d86fee2b3a8bc6657ac1b48a7b2a89770c52e7`; app ABI and firmware header verification PASS.
- Portable full regression suite PASS, including the new five-effects PCM test: each produces nonzero difference from dry; exact dry parity on default OFF, bounded sample output, smooth bypass tail.
- Direct screen exports: 36/36 exact 240x240 monochrome from the native C UI.
- Protected 16 KiB boot header and trusted OTA loader match V1.88 exactly.
- Static RAM margin: 8996 B DSP pool and 4284 B regular RAM; **no hardware CPU or thermal profiling performed**.
- Session-only effects: Transient Shaper, Tilt EQ, Octave Fuzz are not persisted yet.

## V1.16 Sandpaper verification
- FM1 native JieLi pi32v2 crossbuild and repeat build **PASS**, app SHA-256 `4b2fd308ae2e00ca13f9e62039ed692594a77be247db938669e7ce01984cb24a` and 185,736 bytes.
- `tests/test_v116_sandpaper.c`: 95 Hz fundamental ratio about 0.993 with SMOOTH=85, SOFT=0, WARMTH=0; 4.5 kHz attenuated; knob response, 100% input and bypass fade validated. **PASS**. Values are host-model PCM, not physical listening results.
- `bash tests/run_tests.sh` PASS for all existing effects, FM1 project storage, SUB settings, and OTA flash-emulation regressions.
- 37 PNG screens generated from native C source, all exact 240×240 pure black/white.
- FWSC CRC valid, `FM-1_968` identity, SHA-256 `392f3d4934347fec8ad31aeee3a28d0d0d21716a176234ccb2a78e84497daff6`; protected 16 KiB header and trusted OTA loader byte-identical to retained V1.88 baseline.
- DSP pool margin 8,980 B; regular RAM margin 4,276 B. No measured real-time CPU margin.
- Sandpaper parameters are session-only and default OFF. Physical FM1 flash and audio tests still required.

## V1.16 Sandpaper verification
- FM1 native JieLi pi32v2 crossbuild and repeat build **PASS**, app SHA-256 `a61e27c235b99d3c5010726f421ad973c90d1fad36519ccb89e8aab2b06096f8` and 185,736 bytes.
- `tests/test_v116_sandpaper.c`: 95 Hz fundamental ratio about 0.993 with SMOOTH=85, SOFT=0, WARMTH=0; 4.5 kHz attenuated; knob response, 100% input and bypass fade validated. **PASS**. Values are host-model PCM, not physical listening results.
- `bash tests/run_tests.sh` PASS for all existing effects, FM1 project storage, SUB settings, and OTA flash-emulation regressions.
- 37 PNG screens generated from native C source, all exact 240×240 pure black/white.
- FWSC CRC valid, `FM-1_968` identity, SHA-256 `21ab65ef13d6948f34af4889c79adbe3b80138eb2d3e7fa9a7e92161fa88d897`; protected 16 KiB header and trusted OTA loader byte-identical to retained V1.88 baseline.
- DSP pool margin 8,980 B; regular RAM margin 4,276 B. No measured real-time CPU margin.
- Sandpaper parameters are session-only and default OFF. Physical FM1 flash and audio tests still required.

## V1.17 project FX persistence

- Native PJT33 extension of exactly 21 bytes added to the existing project A/B journal; unchanged 33-byte DSP patch record.
- `tests/test_v117_fx_memory.c` tests all sixteen knobs, four enabled flags, 22-bit bypass mask, project switching/isolation, old PJT32 default loading, invalid flags/bypass/knob values and truncated data. PASS.
- `bash tests/run_tests.sh` includes all V1.1–V1.16 regressions. PASS.
- USB/OTA physical reflash and real-time CPU profile: NOT VERIFIED; test on FM1 with a recovery backup before production use.

## V1.17 Sandpaper verification
- FM1 native JieLi pi32v2 crossbuild and repeat build **PASS**, app SHA-256 `4ef0e90ed285cde29f9e353ceab0a9eb6ce3dcd267a0e883b87f7800f54cf7e1` and 186,276 bytes.
- `tests/test_v116_sandpaper.c`: 95 Hz fundamental ratio about 0.993 with SMOOTH=85, SOFT=0, WARMTH=0; 4.5 kHz attenuated; knob response, 100% input and bypass fade validated. **PASS**. Values are host-model PCM, not physical listening results.
- `bash tests/run_tests.sh` PASS for all existing effects, FM1 project storage, SUB settings, and OTA flash-emulation regressions.
- 37 PNG screens generated from native C source, all exact 240×240 pure black/white.
- FWSC CRC valid, `FM-1_968` identity, SHA-256 `f0ef04b4fc670674c1274a9bddf72c180e5a307633b93e2ae8d2cd9f4eed4bdc`; protected 16 KiB header and trusted OTA loader byte-identical to retained V1.88 baseline.
- DSP pool margin 8,952 B; regular RAM margin 4,248 B. No measured real-time CPU margin.
- V1.17: Sandpaper parameters, other new FX and native FX bypass now persist in PJT33 project data. Host-only validation; physical FM1 still required.

## V1.18 full-chain stability verification
- JieLi pi32v2 ELF crosscompile and app validation PASS; 186,724-byte app; SHA-256 `2b24b8385b07437d68d38a405e425855ce32144f3ba0f17bf1873d37191af9d0`. Repeated build reproduced exact package contents.
- `bash tests/run_tests.sh` PASS including PJT33 12-slot save/restore and legacy project migration.
- `tests/test_v118_full_fx_stress.c`: all 10 RUPTURE FX maximum (including 100% wet on chorus/delay/reverb, compressor, wavefolder, transient, tilt EQ, octave fuzz, Sandpaper) over 220,500 output PCM frames. Peak 26,038/32,767, no hard fullscale clip, no systematic unintended silence, test PASS.
- Run with `-fsanitize=undefined -fno-sanitize-recover=all`: PASS. Does not model all possible external hardware MIDI/USB interrupts or real sample-rate timing.
- Dry no-effect PCM remains bit-exact; old UI, CXS, SUB, white-LED and MIDI tests remain passing.
- `.fwsc` SHA-256 `40ea8f30c5f2022bc4d322a1c857c976cce48ddc190d288d9a3a6a5ac3f0e2f2`, product `FM-1_968`, CRC PASS. Protected 16 KiB header + OTA loader byte-identical to archived V1.88.
- RAM margin .pool 8,944 B, data+bss 4,248 B. Full-load FM1 CPU wall timing, DMA and audible aliasing not hardware validated; residual overruns remain possible until an on-device soak test.

## V1.19 native DMA instrumentation and scope priority
- Native FM1 pi32v2 build PASS; app 187,260 B, SHA256 `30a9e9ab2241d9e65a8f3b24148ae25f422079a23d197bd86756808259d98b00`. Second build byte-identical.
- `bash tests/run_tests.sh` PASS, including 12-project NOR torn-write tests, original dry CXS PCM and 10-FX maximum all-effects stress.
- `tests/test_v119_dma_diagnostics.c` PASS: 75% full-load scope cutoff, hysteretic 16-point UI pressure cutoff, correct DMA ownership preflight, timer rollover and correctly clamped nested-ticks accounting.
- New RUPTURE AUDIO DIAG screen from exact native C renderer: 38 screens, 240x240, black/white. GLO → AUDIO DIAG exposes maximum whole ISR, synth+FX render, DMA+USB publish and minimum slack, plus LATE/GUARD and visualization-skip counters.
- Guard for expired DMA halves and protective audio stop remain fully enabled; on hardware underruns may still occur. Not measured: actual ISR wall time, nested USB preemption or audible artifact rate on FM1.
- App static RAM margin: pool 8,944 B, normal 4,216 B, noinit 3,500 B, UAC 5,684 B.
- FWSC identity FM-1_968, SHA256 `11c60d6f59f68f4df037c462b9a6cb8e1724c14944115d6ebaea6f1614424979`, stock protected 16 KiB and Felucca OTA loader byte-identical.
