# RUPTURE V1.15: FX DSP and static memory audit

Measured from the compiled pi32v2 ELF, *not* live hardware CPU profiling.

| Linker memory | Capacity | Used | Unallocated |
|---|---:|---:|---:|
| DSP `.pool` | 344,064 B | 335,068 B | 8,996 B |
| Regular `.data + .bss` | 98,304 B | 94,020 B | 4,284 B |
| Retained `.noinit` | 15,696 B | 12,196 B | 3,500 B |
| USB `.uac_ram` | 8,192 B | 2,508 B | 5,684 B (reserved for USB) |

Compiled application size 185,032 B; application-space margin 396,532 B. This does *not* measure available realtime CPU cycles. Relative to V1.14, the five new effect routings add no circular audio buffers, only bounded envelope and filter states and several byte-sized UI controls. FX global processing remains once per final mono mix.

## Effects actually implemented

- Compressor: existing Q15 envelope follower, threshold, ratio, attack, release. It uses existing PJT32 FX fields.
- Wavefolder: existing bounded symmetric wavefold, fold, symmetry, tone, mix. Existing PJT32 FX fields.
- Transient Shaper: dual magnitude envelope tracking, bipolar attack/sustain, sensitivity and wet mix. Session-only settings; short slew for transitions.
- Tilt EQ: low/high split, bipolar tilt, cutoff-frequency approximation, wet mix, output gain. Session-only.
- Octave Fuzz: nonlinear predrive plus full-wave rectification (prominent octave-*up* partials), DC rejection, tone filtering, wet mix. Session-only. Not an octave-*down* pitch-tracking algorithm.

All five are bypassed by default; dry host audio matches input exactly. The existing CXS and suboscillator algorithms are unchanged. Under heavy loads the DSP can still exceed realtime CPU limits; this is **not tested on physical FM1 hardware**. Higher gain settings can trigger output limiting and aliasing; check on speakers and headphones before assuming clean gain.


## V1.16: Sandpaper added to the live compiled native ELF

| Segment | Free after V1.16 |
|---|---:|
| DSP `.pool` | 8,980 bytes |
| Regular RAM `.data + .bss` | 4,276 bytes |
| Retained `.noinit` | 3,500 bytes |
| USB dedicated `.uac_ram` | 5,684 bytes |

App: 185,736 bytes; application capacity margin 395,828 bytes.
Sandpaper has three bounded IIR filter state integers plus a mix scalar, no sample ring, no heap and no extra file/flash storage. Actual ISR CPU usage and sound have **not** been measured on an FM1.


## V1.16: Sandpaper added to the live compiled native ELF

| Segment | Free after V1.16 |
|---|---:|
| DSP `.pool` | 8,980 bytes |
| Regular RAM `.data + .bss` | 4,276 bytes |
| Retained `.noinit` | 3,500 bytes |
| USB dedicated `.uac_ram` | 5,684 bytes |

App: 185,736 bytes; application capacity margin 395,828 bytes.
Sandpaper has three bounded IIR filter state integers plus a mix scalar, no sample ring, no heap and no extra file/flash storage. Actual ISR CPU usage and sound have **not** been measured on an FM1.

## V1.17 FX snapshot allocation

PJT33 adds 21 serialized bytes per project, inside the already allocated A/B project banks. The new project working state costs 24 bytes (compiler alignment) in RAM. No new ring/delay buffers or DSP algorithms. On-device worst-case runtime and ISR CPU cost remain unmeasured.


## V1.17: Sandpaper added to the live compiled native ELF

| Segment | Free after V1.17 |
|---|---:|
| DSP `.pool` | 8,952 bytes |
| Regular RAM `.data + .bss` | 4,248 bytes |
| Retained `.noinit` | 3,500 bytes |
| USB dedicated `.uac_ram` | 5,684 bytes |

App: 186,276 bytes; application capacity margin 395,288 bytes.
Sandpaper has three bounded IIR filter state integers plus a mix scalar, no sample ring, no heap and no extra file/flash storage. Actual ISR CPU usage and sound have **not** been measured on an FM1.

## V1.18: compiled full-FX stress and memory

| Segment | Static remaining |
|---|---:|
| DSP `.pool` | 8,944 bytes |
| `.data + .bss` | 4,248 bytes |
| `.noinit` | 3,500 bytes |
| `.uac_ram` (dedicated USB) | 5,684 bytes |

Application 186,724 B. The ceiling DSP update adds no audio rings, no malloc, and no new flash sectors. The output knee has only shifts/adds and hard caps at 28,000 Q15 for active wet signal. Critical sample multiplies in the new FX and delay/reverb feedback are now 32-bit with explicit operand bounds. Two fixed-memory input poles filter strong Wavefolder and Octave Fuzz drive above the upper bass band; true bandlimited oversampling is not implemented.

The host 5-second 10-FX stress yielded peak 26,038 Q15, no full-scale clips or systematic dropouts. UBSan host execution PASS; full legacy tests PASS. Host PCM processing time **does not** measure native FM1 ALNK DMA deadline timing. Two cheap pre-nonlinearity Q15 lowpasses reduce high-frequency drive-fold alias products without 2x oversampling. The final output limiter does not remove aliasing that a nonlinear wavefolder or fuzz stage has already generated; hardware oversampling is not implemented.

## V1.19 realtime DMA telemetry / build budget

The new read-only ISR diagnostic counters use scalar memory, not PCM buffers, and are sampled at one interrupt per 128 frames. Scope visualization can defer under pressure; audio processing, FX and USB transport are unchanged. RAM available after the native linker build: .pool 8,944, data+bss 4,216, noinit 3,500, UAC SRAM 5,684 B. Static values do not indicate realtime CPU headroom. FM1 hardware diagnostic measurements are still required.
