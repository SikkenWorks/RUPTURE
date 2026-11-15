# RUPTURE V1.85 — Realtime Reliability / release verification

## Scope and cause

Baseline: **V1.84.2**. Physical FM1 DMA LATE and GUARD counters can increase under rapid notes. The prior USB input ISR allowed up to **64 MIDI packets** to be drained and dispatched inside one 128-frame ALNK0 audio deadline. Independently, outbound panel-MIDI scanned all **27 keys twice** per audio block even when no state changed. The audio ISR already classified `render` and `publish` peak times, but did not show the pre-render control/MIDI time or a separate post-publish late count.

These are **source-confirmed work paths**, not proof that they caused a specific physical underrun or speaker plop. V1.85 targets bounded worst-case work and observable diagnostics without changing PCM DSP or weakening safety guards.

## Code changes

1. `native_midi_rt_budget.h`, `audio_target.c`: process **at most 32 USB MIDI events** per 128-frame audio half. Other packets stay in the original FIFO queue. **No events are discarded by the budget** and ordering remains unchanged. The existing bounded 64-packet ring and overflow panic policy are retained. Under sustained bursts exceeding capacity, the original overflow policy can still panic; very large one-shot bursts can acquire extra block latency (up to approximately one half-block for packets 33–64). This is a deliberate workload tradeoff, not a guaranteed no-drop MIDI scheme.
2. `native_panel_midi.c/.h`: one fast state comparison avoids redundant two-pass scans of all 27 keys. When any USB emit failed due to TX backpressure, `retry_pending` prevents the shortcut so that Note OFF/ON retries still occur. Reconnect and octave changes continue to send the original packet sequences.
3. `audio_target.c`: when there is unserviced USB-MIDI backlog, the *optional* waveform scope snapshot is skipped for that block. This does not affect note or audio processing. No additional allocation or high-priority IRQ is added.
4. `audio_target.c`, `bringup.c`, `rupture_ui.c`: new read-only **SEL-toggle detail page** under GLO → AUDIO DIAGNOSTICS. New fields are CONTROL MAX (% of DMA period spent before rendering), NESTED MAX (% TIMER5 nesting), SLACK LAST (% most recent DMA remaining), MIDI BATCH (peak processed in a block), MIDI QUEUE (peak queued before processing), MIDI DEFER (number of blocks leaving MIDI packets queued), GUARD (pre-publish ownership failures), POST LATE (post-publish deadline failures). Original eight metrics remain on the first page. `LATE`, `GUARD` and `POST LATE` remain cumulative across audio restarts, so **LATE = GUARD + POST LATE** unless integer counters wrap; other maxima/resettable statistics have the original or new audio-start semantics.
5. V1.85 metadata and documentation, no project format change.

## Completed checks

| Test | Result |
|---|---|
| Native JieLi `pi32v2` crosscompile, storage+updater+UAC48 | **PASS** |
| Native application ABI, entry stub and slot size audit | **PASS** |
| Deterministic identical rebuild | **PASS** |
| 120,000 randomized panel states versus original V1.84.2, including USB full queue/retries | **PASS**, exactly 52,701 emitted USB MIDI packets in matching order |
| USB MIDI burst budget, queues 0–64, exact FIFO and no budget loss | **PASS** |
| Original 1-bit screen/native UI tests | **PASS** |
| V1.84 page LED/button ownership and encoder tests | **PASS** |
| V1.84.1 LED current ramp and V1.84.2 PC6 analog-enable separation | **PASS** |
| Mono fixed-divider and existing rapid-note PCM parity | **PASS** |
| V1.82 Welcome/CRT journal migration, project persistence, interrupted writes | **PASS** |
| Three real reverb algorithms, decay/size and 2,000,000 hot-switch samples | **PASS** |
| Preserved audio core, effects, project codec, OTA, hardware LED/PC6 modules | **PASS: 11 native source SHA-256 comparisons to V1.84.2** |
| Firmware FWSC verifier + OTA loader and first 16KiB boot header | **PASS: protected bytes identical** |
| Firmware / WebInstaller / GitHub index byte parity and SHA-256 | **PASS** |
| Full historical regression suite | **NOT EXECUTED**; selected related tests only |
| Real FM1 long-duration fast-note + USB+TRS + LCD DMA stress | **NOT TESTED** |
| Actual speaker-plop electrical measurement | **NOT TESTED** |

## Firmware integrity

- App size: **348,616 bytes**, remaining application slot capacity **232,948 bytes** (not DSP headroom).
- App SHA-256: `30c18e829ec5818b375d184f71f80dfe4decdb38d655b72f3587098789a0b733`
- Firmware `.fwsc`: **610,086 bytes**, SHA-256 `9abbfce69af4d515ada7132b7faa2ffac6b5a46f34b2f19862f30dd7bf31c2f1`
- Protected boot header (first 16,384 bytes) SHA-256 `a65e0e1a2fad1db943c00b529b45553c524257146145db61b69628cefcf2b2c9`, byte-identical to V1.84.2.
- Trusted OTA loader (6,930 bytes) SHA-256 `d71d1b7c12adede951605e758a3580dacccedc427d49e1fceaf51decccc68a4b`, byte-identical to V1.84.2.

## Hardware acceptance steps

Back up projects and retain the validated Pico recovery route. With initial speaker volume low, power-cycle the FM1, go to **GLO → AUDIO DIAGNOSTICS** and record *LATE* and *GUARD*. Press **SEL** for the new detailed screen. Check the new MIDI DEFER/BATCH/QUEUE and CONTROL MAX data while (a) playing very fast on the built-in keys, (b) streaming USB-MIDI bursts, and (c) activating both SUBs, DRIFT, maximal reverb/FX and LCD animation. Verify that LATE/GUARD never increase. Monitor whether `LATE - GUARD == POST LATE` across recovery restarts. If new LATE/GUARD events occur, record both diagnostic screens and note whether MIDI QUEUE/CONTROL MAX or RENDER/PUBLISH dominate. Test TRS-MIDI concurrently, disconnections and PC6 speaker activation separately.

**Important:** This release is **not** a physically verified no-underrun firmware. CPU performance gains are inferred from eliminated scans and bounded USB queue work; there are no trustworthy FM1 hardware cycle-speedup measurements. Prior analog-speaker pop mitigation remains as-is and also requires physical validation.
