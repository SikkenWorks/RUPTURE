# RUPTURE V1.94, FX independence and live reliability

## Result

The firmware retains exactly the same large audio PCM ring allocations as
V1.93 (delay 16,384, chorus 2,048 and reverb 4,096 int16 samples). New small
per-slot state and four-knob parameter banks use extra **control** RAM, but no
additional time-effect audio buffers. The original single-instance FX
transfer is sample-exact, verified across all ten effects in
`tests/test_v194_unique_pcm_parity.c`.

## Relative host CPU benchmark (x86, not FM1)

The following measurements use the existing `bench_v193_repeat_fx.c` with
300,000 stereo frames per configuration, compiled with `-O3`. They are rough
diagnostics, sensitive to host scheduling and unrelated to the pi32v2 DMA
budget. Repeated active processors inherently perform additional work.

| Case | V1.93 ns/frame | V1.94 ns/frame |
| --- | ---: | ---: |
| 8 empty | 58.21 | 62.32 |
| 1 reverb | 189.08 | 189.66 |
| 8 reverbs, shared tank | 272.48 | 311.39 |
| 1 delay | 102.77 | 90.01 |
| 8 delays, shared history | 242.90 | 260.59 |
| 1 distortion | 109.54 | 142.73 |
| 8 distortions | 380.50 | 377.01 |
| 1 wah | 125.29 | 187.66 |
| 8 independently computed wahs | 225.34 | 709.54 |

All numbers are from one run, **not a performance guarantee**. The largest
relative increase is eight Wah instances, because V1.93 only computed its
envelope once whereas V1.94 gives each Wah a genuine independent envelope.
This is the real tradeoff between independent FX and a constant CPU budget.

## What is and isn't independent

- All ten types: four separately editable and serializable parameters per slot.
- MOSFET, compressor, tilt, fuzz, sand, distortion and wah: isolated runtime
  state per slot (or no state for MOSFET).
- Chorus: independent rate/depth/mix taps; shared original mono PCM history.
- Delay: independent time/mix taps; all per-slot feedback values contribute
  to one averaged shared PCM feedback write. This is audibly responsive to
  the second Feedback knob, but is not a fully independent echo loop.
- Reverb: independent wet level. One shared FDN tank means only the first
  active reverb can control its engine, size and decay. Values in additional
  reverb slots are stored independently but not independently processed.

No physical FM1 was connected to this build. Verify the on-device LATE,
GUARD, POST LATE, NESTED MAX and DMA slack counters under the intended
worst-case patch before using this firmware live. Until then V1.94 is a
**software test candidate**.
