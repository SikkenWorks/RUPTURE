# RUPTURE V1.91 host performance and FM1 acceptance criteria

## Scope

This is a **host-side microbenchmark, not a measurement of FM1 CPU cycles or audio DMA timing**. It isolates the post-mix FX routine and omits oscillator, MIDI, LCD, USB and storage load. Host timings are affected by CPU clocks and scheduling; absolute values do not predict FM1 performance.

Benchmark program: `tests/bench_v191_fx_slots.c`, compiled with GCC `-O2` on a Linux x86 host. 300,000 PCM frames per case. Three consecutive runs after the final V1.91 audio build:

| FX case | Run 1 ns/frame | Run 2 ns/frame | Run 3 ns/frame |
|---|---:|---:|---:|
| Eight empty slots | 12.24 | 12.04 | 11.87 |
| Reverb only | 35.80 | 36.11 | 36.53 |
| Eight active slots | 123.99 | 118.29 | 118.28 |
| V1.89 legacy FX + voice drive | 106.36 | 101.98 | 104.91 |
| Seven slots, no MOSFET | 111.97 | 110.86 | 117.74 |
| MOSFET only | 27.04 | 25.40 | 27.55 |
| V1.89 legacy FX without drive | 109.04 | 101.24 | 103.17 |

Median **eight active**: 118.29 ns/frame. Median **V1.89 legacy + drive**: 104.91 ns/frame. In this particular comparison the eight-FX case is approximately **12.8% heavier**, not equal or lighter. Eight empty slots are substantially cheaper than active V1.89 effects, as expected.

These are different signal paths, with different audible processing and checksums. This is **not** an identical-workload apples-to-apples result; it is a practical indicator of extra maximum-insert cost. A previous V1.90 run reported approximately 141 ns/frame for full eight-FX processing, so V1.91 moves in the right direction without eliminating the load difference.

## Release safety status

- Verified in source: zero heap allocations in the new slot DSP, limited static MOSFET table, bounded dry-state flush, atomic nine-byte FX routing update during menu actions.
- Verified in native cross-build: firmware fits the FM1 app space with ~187 KiB headroom; protected bootheader and trusted OTA loader match V1.90 byte-for-byte.
- Verified on host: full eight-FX arithmetic and aggressive reorder/bypass stress, MOSFET bounds/monotonicity, legacy FX PCM parity, PJT35/older project codec and full regression suite.
- **Not verified:** peak ISR cycles on the real pi32v2, DMA underrun count while performing and manipulating all eight inserts, temperature/power/USB-MIDI interaction, speaker acoustics, power-cycle restoration, actual device OTA installation.

## Physical FM1 go/no-go checklist

Do not use this test candidate as a sole live-performance firmware until:

1. Back up projects and ensure you have a working recovery route.
2. Verify idle and one-FX audio first, with a sustained low bass note and rapid octave/legato slides.
3. Add all eight FX, play sustained and staccato notes at maximum intended gain, and monitor audio DMA overrun/deadline diagnostics for **zero** errors.
4. Reorder, bypass and re-enable slots during sustained notes. Check for pops, silent audio, stuck notes or changed levels. Repeat over a prolonged session at normal stage volume and USB/TRS MIDI load.
5. Save a project, restart, reload and verify each slot, FX order, ON/OFF bit, all parameters and GLIDE settings. Test OTA and cold-start behavior separately and cautiously.

**Accept for live use only after those physical-device checks are completed.** Until then V1.91 is a software-verified release candidate, not a guaranteed live-safe build.
