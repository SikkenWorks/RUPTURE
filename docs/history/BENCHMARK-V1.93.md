# RUPTURE V1.93: repeated-FX cost, sonic integrity and live safety

The following are **Linux x86 host** microbenchmarks of the post-mix FX loop only, built with `gcc -O2`. Each case renders 300,000 stereo frames. This **does not measure** FM1/pi32v2 CPU cycles, deadline misses, USB/SD usage, LCD work or actual sound quality. Clock speed fluctuated; do not use absolute times as hardware estimates.

| FX chain (one FX type) | Host pass 1 ns/frame | Host pass 2 ns/frame |
|---|---:|---:|
| 8 empty slots | 13.35 | 11.73 |
| 1 Reverb | 42.13 | 37.06 |
| 8 Reverbs, shared tank | 89.77 | 73.00 |
| 1 Delay | 36.71 | 29.28 |
| 8 Delays, shared tank | 71.55 | 65.34 |
| 1 Distortion | 33.59 | 27.08 |
| 8 Distortions, shared tone history | 99.05 | 91.63 |
| 1 Wah | 33.77 | 32.10 |
| 8 Wahs, shared filter state | 66.65 | 66.63 |

Before the repeated-Wah filter-state optimization, eight Wah inserts measured about **143 ns/frame** in the same benchmark; after applying the wet return repeatedly while updating the envelope/filter only once per frame, they measured about **67 ns/frame**. This is an isolated host improvement, not FM1 headroom evidence.

## Audio fidelity contracts

- The **pre-existing, unique-type** V1.92 chain is bit-for-bit PCM-identical in the test oracle, including Distortion/Wah reorder hashes `3479413712` and `4007301924`.
- Two or more identical effects do alter the actual audio PCM. The original insert slot is not silently replaced or bypassed.
- The FX chain still has eight slots, no additional PCM ring buffers and no new synth voices. Its serialized PJT35 record size is unchanged.
- Slot bypass fades are independent. Each time-based ring, Wah envelope, dynamics detector and tone state advances at most once per input PCM frame regardless of repeated slots. This prevents delays playing at half-duration and filter envelopes running eight times faster than intended.
- Repeated instances **share their type's knob parameters and the heavy effect's internal state**. In particular two reverbs do not have separate room histories. This is a resource-saving layering approximation, not two fully independent professional rack units.

## What has not been proven

The user's desired **zero additional CPU cost for arbitrary repeated effects is not mathematically possible** while each enabled slot modifies the signal. Eight active inserts still perform more mixing and clipping operations than a single insert, as the benchmark shows. The native binary fits in the existing app space, but no FM1 DMA LATE, GUARD or MIN SLACK measurements are available. This software remains a **test candidate, not yet approved for live use**.

On hardware, play both SUB oscillators with sustained notes while rotating knobs, rapidly switching slot ON/OFF, rearranging duplicates and using USB/TRS MIDI. Observe audio diagnostics for continuously zero LATE/GUARD and adequate MIN SLACK after a long rehearsal. Also compare listening quality, power-cycle loading and normal versus maximum FX chains. Keep a backup and the known working recovery method available.
