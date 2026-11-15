# RUPTURE V1.92: FX performance and acceptance criteria

This is a **Linux x86 host microbenchmark** of the post-mix FX routine, not a pi32v2/FM1 cycle measurement. It excludes oscillators, MIDI, display, USB, audio DMA, flash and amplifier. The CPU clock varied across the three consecutive runs. The numbers are only useful for identifying possible worst-case work; **do not infer live stability from them**.

`tests/bench_v192_fx_slots.c` compiles `native_fx.c` at `-O2` and runs 300,000 PCM frames per case.

| Case | Run 1 ns/frame | Run 2 ns/frame | Run 3 ns/frame |
|---|---:|---:|---:|
| Eight empty slots | 13.42 | 12.16 | 12.52 |
| Distortion alone | 26.76 | 27.27 | 27.61 |
| Wah Wah alone | 27.00 | 27.92 | 28.45 |
| Eight old FX types | 116.92 | 116.14 | 118.66 |
| Eight FX including both new types | 131.42 | 125.42 | 124.61 |
| V1.89 legacy FX plus drive | 101.88 | 100.49 | 102.26 |

The two eight-insert cases are not equivalent: the latter substitutes the new Distortion and Wah for Chorus and MOSFET. Their audio checksums are different. The median for a maximum chain containing both new effects is **125.42 ns/frame**, versus **116.92 ns/frame** for the previous collection of eight, approximately **7.3%** extra isolated FX work. The previous legacy plus drive path is approximately **101.88 ns/frame** in this run. A fused legacy-order COMP/TILT/FUZZ/SAND path prevents accidental duplicate Wah processing before its slot and substantially reduces peak CPU relative to an unfused path. It is still a reason to measure actual FM1 DMA slack before live use. It does not imply a specific audio underrun rate on the FM1.

## What was done to protect real-time behavior

- Exactly eight slot positions, ten available types. A type appears no more than once in one chain.
- Distortion uses signed 32-bit half-scale fixed-point gain/clip/tone arithmetic with no 64-bit multiplication or new delay line.
- Auto-Wah reuses an existing envelope follower and two one-pole filter states, with bounded coefficients and resonance. Both new wet-mix controls slew to avoid hard steps under fast knob movements.
- Neither new algorithm runs when omitted/bypassed, and a parameter mix of zero skips its processing.
- Insert/remove/reorder updates are published under a short interrupt guard. Orphaned fade states are cleared in that UI-only section to keep the dry fastpath available.
- Existing Glide, project archiving, Web MIDI connection, OTA loader and flash-ID handling are unchanged.

## What remains to qualify for live use

1. On a real FM1, check zero audio DMA LATE/GUARD errors when continuously playing a sustained bass note with all eight active FX, maximum intended resonance/drive, and USB or TRS MIDI.
2. Edit all effect parameters while performing, rotate K4 to bypass/re-enable, reorder the slots, and listen for pops, interrupted notes, unexpected volume and crackle.
3. Let a complex patch run for a prolonged rehearsal; monitor battery/power and display load.
4. Back up a project, save/reboot/reload and check every FX parameter and slot order. Never downgrade to V1.91 before removing the new FX IDs and backing up.

**Status: software-validated test candidate only. There is no physical hardware evidence yet that V1.92 has enough real-time headroom with eight expensive inserts.**
