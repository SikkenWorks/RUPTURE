# RUPTURE V1.78 — Native reverb engineering and verification

Date: 2026-11-04. Baseline: complete RUPTURE V1.77 source project. **Status: successful native cross-build and targeted host tests. Physical FM1 performance and listening not yet verified.** The historical suite stops at the previously unresolved V1.27 pixel assertion; no full-suite success is claimed.

## Actual algorithms, not presets

The old K1 selector in `bringup.c` merely loaded three SIZE/DECAY combinations into one effect algorithm. V1.78 eliminates this coupling and passes a real algorithm code to the native DSP. `native_fx.c` defines three separate per-sample processing topologies, with only the selected one executed:

- SPRING (`ru_reverb_spring`): 29-sample dispersive allpass stage, alternating-sign three-tap feedback and filtered resonance. This is a lightweight digital spring-like algorithm, **not** a physically modeled mechanical spring.
- HALL (`ru_reverb_hall`): Schroeder-style input diffusion, filtered recursive three-tap late tank and early stereo returns.
- ROOM (`ru_reverb_room`): discrete sparse early-wall reflections from different taps, followed by a short damped comb feedback tail. It does not run Hall diffusion.

All branches use the existing 4096-sample `s->reverb` ring, existing global 64-sample `space_diffuser`, and `s->space_damp_state`; only one runs after the summed synth voices. `sizeof(struct etch_native_fx)`, DSP buffer allocation, DMA buffers and voice count do not increase. Coefficient and delay-address divisions are computed only when settings change. `MIX` is one common true wet/dry interpolation: 0 = unprocessed dry, 100 = wet-only. The native ceiling for active wet effects is ±28,000 (headroom limiter).

## UI/persistence compatibility

- FX/REVERB K1 selects actual topology, **without altering K2 DECAY or K3 SIZE**. K4 continues controlling MIX.
- Preserve existing 1-byte `routing` project field: bit 0 = serial/parallel; bits 1–2 encode reverb type (0 Hall, 1 Spring, 2 Room). Values 0/1 in legacy saved projects decode as Hall. The codecs now validate 0–5; 6/7 remain rejected. Existing PJT13/PJT34 serialized length and CRC remain unchanged.
- Global and per-preset FX decode and reload their reverb type. Both legacy and PJT34 encode/decode tests pass.
- **Downgrade warning:** V1.77 and older have strict `routing <= 1` validation; newly saved projects containing Spring/Room type bits may be rejected by those old firmwares. Back up projects before installing or downgrading; no automatic reverse migration is claimed.
- Device firmware identity and USB update protocol are unchanged.

## Native test results

| Check | Result |
|---|---|
| Cross-compile, Jieli pi32v2 with storage/updater/USB 48 kHz | PASS |
| Application XIP audit and entrypoint | PASS |
| Three-branch audio PCM uniqueness, same reverb controls, 70,000 samples per mode | PASS |
| True 100% wet processing and nonzero silent-input tails | PASS |
| Peak limiter bound, all 3 modes | PASS |
| Full FX stress test, ten effects, clipped=0, peak=26,346 | PASS |
| Dry sample-exact bypass and smooth reverb bypass | PASS |
| PJT13 and PJT34 global/per-preset algorithm save/load | PASS |
| Legacy 0/1 route decoding (Hall) and route/engine independence | PASS |
| Existing RUPTURE V1.77 SAW and reverb sound tests | PASS |
| Legacy V1.33 sequences, PJT34 EQ2 and PJT33 FX-memory | PASS |
| Existing reverb/FX tests V0.9, V1.14, V1.18, V1.22 | PASS |
| Full historical suite | Incomplete: stops at `tests/test_v127_native_ui.c:19` hard-coded pixel assertion |
| FM1 physical listening and real DSP/DMA load | Not performed |

### Distinct output measurements

All three tests use 44.1 kHz, 110 Hz signal for 11,025 samples followed by silence, SIZE=24,000, DECAY=26,000, MIX=100%, all other effects off. Numbers are host PCM samples and not hardware loudness measurements.

| Engine | Wet RMS during note | Tail RMS after note | Stereo side RMS |
|---|---:|---:|---:|
| SPRING | 3,205.8 | 1,240.9 | 4,155.4 |
| HALL | 6,701.4 | 1,408.1 | 4,303.3 |
| ROOM | 5,612.1 | 846.4 | 1,044.5 |

Distinct PCM hashes are generated and checked for each mode in `tests/test_v178_three_reverbs.c`.

### Realtime constraints and performance limits

The complete reverb-only host benchmark gave approximately 37–50 ns/sample for the old single engine and 39–49 ns/sample across the new engines on this container's CPU; measurements varied between runs and **are not a reliable target-hardware CPU guarantee**. There are no additional voices, DSP buffers or parallel reverb engines. The mode dispatch and distinct math can have different small instruction costs. Real pi32v2 cycles, IRQ deadlines and USB audio at high load still need hardware verification. Avoid claiming exact cycle parity.

### Update and recovery integrity

The exact `RUPTURE-V1.71.fwsc` reference package was used to rebuild the controlled package. The first 16 KiB boot header, trusted `ota.bin` loader and non-app flash contents are byte-identical to the reference. `ota.c`, `audio_core.c` and hardware startup sources are unchanged from V1.77. The WebInstaller and generated website embed the exact same verified firmware bytes as the standalone `.fwsc` release. Installer checks retain the original identity, CHECK/WRITE and cryptographic validation code. **These tests do not constitute a successful actual FM1 OTA flash or recovery validation.**
