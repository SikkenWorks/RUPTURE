# RUPTURE V1.69 — DSP and UI Performance Verification

## Status

**Native FM1 build passes; targeted host/audio/OTA checks pass. Hardware validation is outstanding.** These tests cannot prove the actual FPS or DSP percentage on an FM1, and cannot prove physical cross-firmware / original V15 restore.

## Byte-exact audio and UI equivalence

- Host PCM reference: 16 mixed FX scenarios × 50,000 stereo frames = **800,000 stereo frames**. The baseline V1.68 native FX implementation and the V1.69 source produce identical 64-bit rolling checksums for every scenario. Scenarios include compressor threshold/ratio/attack/release parameter automation, bypass transitions, EQ, ring, folder, delay, chorus and reverb. The 12 V1.40 matrix scenarios also match V1.68 exactly (24,000 frames per scenario).
- Native 1-bit LCD: **42 full 240×240 screen states** render with exactly the same framebuffer hash as V1.68, including FX, knob focus, EQ and sequencer. Knob circle uses the same original **332 pixels** with no new overlap. This equivalence excludes the intentionally updated V1.69 SYSTEM INFO version label.
- FX-state footprint: **47,956 bytes before and after**, on host compiler. The compiler's FM1 memory audit also passes. The compressor's cached values reuse previously unused alignment bytes and the inactive RUPTURE-only legacy EQ HPF field.
- Extra FX stress: existing 220,500-frame heavy-FX regression passes with zero clipped frames and exact dry bypass.
- Original source art: all 60 ENV/LFO/vulcano assets match the embedded firmware sprite data, and V1.66 note-gate animation/shake test passes.

## Scheduling and input verification

- Replaces 2,025 per-knob radius tests with 332 original visible knob pixels per draw; the needle and center dot remain identical.
- When DMA is healthy, knob and panel animation repaint is requested at 25ms (40 FPS target) and remains independent of high historical DSP MAX. Immediate input redraw requests remain unblocked. At one or repeated actual DMA misses, the original audio-first fallback (50ms / 100ms requests) remains.
- Knob easing uses elapsed milliseconds and catches up after missed frames. Text values reflect the actual parameter independently from the needle.
- Existing V1.67 all-knob-focus suite passes on FX, EQ, SUB and other pages. Existing V1.68 sequencer step 1 deletion, playhead rendering and 128-step HOLD safety pass; welcome/setup and project-undo tests pass.

## Firmware & OTA safety

- Native application built with `--storage --updater --usb-audio-48k`, firmware identity `FM-1_968`; native ELF/application verifier passes.
- New V1.69 `.fwsc` container verifier and embedded WebInstaller/site SHA-256 parity pass.
- Protected 16 KiB boot area **byte-identical to V1.68**.
- Trusted OTA loader **byte-identical to V1.68**.
- Critical native `ota.c`, flash HAL, `audio_core.c`, `audio_target.c`, project codec and wave-store source **byte-identical to V1.68**.
- Native V1.43 OTA replay using the actual V1.69 `.fwsc` passes: valid app-only commit, bounded old-loader reclaim, refusal of unknown staging, corrupt loader, corrupt header CRC, and mismatched boothead without unsafe flash erase.

## Remaining physical tests (NOT PASS)

1. Verify actual DSP NOW/peak behaviour and DMA late/guard counters on the device at idle, full FX, USB audio and during four-knob sweeps.
2. Confirm hardware LCD FPS, knob easing, and all four loop/shake animations while holding notes and recording a sequence.
3. Confirm storage writes, power-cycle persistence and project recall.
4. Confirm RUPTURE-to-other-firmware and official stock V15 recovery through the real installer/recovery procedure with backup available.
5. Run prolonged hardware soak with headphone/USB capture and verify no audio artifacts when LCD redraws are heavy.

**No hardware connected, no physical-flash claim.**
