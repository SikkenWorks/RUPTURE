# RUPTURE V1.74 — Native Build & Verification

## Native changes

- Live recording now tracks the physical key-down duration against the actual audio sequencer playhead in the 25 ms UI/control thread; `Note OFF` finalizes the stored note length. Existing longer overdub durations are never shortened. The audio callback/voice allocator are not changed.
- Holding physical SEQ and turning PRESETS rotates all steps by one step per encoder detent (counterclockwise earlier, clockwise later). The pattern wraps rather than dropping boundary steps. NOTE durations, local HOLD entries, linked HOLD provenance (`hold_ago`) and parameter locks are retained. The inactive native patch buffer is populated before the new pattern pointer is published with a brief IRQ mask. While not holding SEQ, PRESETS is the existing BPM knob.
- Long SEQ no longer opens the sequence slot browser. SAVE on SEQUENCER still opens the named sequence slots.
- First-run wizard remains exclusive to an uncompleted setup. A returning unit with saved `welcome_done` now displays a short, read-only Welcome to RUPTURE overlay during boot. The saved completion flag and stored data are never overwritten by this preview.
- HELP (12 cards), the 43-page Quick Tour SEQUENCER lesson, on-device sequencer footer, README, cheat sheet, website and CHANGELOG now document the controls.

## Executed checks

| Check | Result |
|---|---|
| JieLi pi32v2 compile (`--storage --updater --usb-audio-48k`) | PASS |
| Native entry point and application size audit | PASS: 0x02000120, 343708 bytes of 581564, headroom 237856 bytes |
| `test_v174_seq_shift_sustain` | PASS: held note length, Note OFF, wrap, overdubs, 8 keys, circular shift, HOLD links, locks |
| `test_v174_welcome` | PASS: separate returning and first-run 1-bit UI phases |
| `test_v172_metro_and_step` | PASS: independent click, complete step clear, linked HOLD cleanup and local locks |
| `test_v168_seq_entry_playhead` | PASS: notes, HOLDs, playhead and no lock loss |
| `test_v170_env_lfo_hold` | PASS: settled ENV debris, continuing shake, closing, LFO loop |
| `test_v171_tutorial` | PASS: entire 43-page Quick Tour and first-run navigation |
| `test_v123_help_cards` | PASS: 12 scrollable HELP cards |
| `test_v160_welcome` | PASS: safe first-run prompts, explicit format confirm, CRT options |
| `test_v173_record_controls` | PASS: PRESETS BPM, ALGORITHM click volume only during REC |
| `test_v170_hold_png_parity.py` | PASS: existing 8 ENV PNGs, LFO PNGs, native sprite bytes |
| `test_v173_mapping_docs.py` | PASS: native routing, tutorial, HELP and documentation controls |
| Website firmware/data/hash parity | PASS: .fwsc, WebInstaller, single-file index and metadata agree |
| Bootheader and OTA loader comparison against V1.71 | PASS: exact same first 16 KiB and `ota.bin` |
| Core DSP/OTA/storage/metro/patch swap implementation compared to V1.73 | Byte-identical: `audio_core.c`, `ota.c`, `wave_store.c`, `native_performance.c`, `rupture_metronome.h`, `native_patch_swap.h` |
| Full historical `tests/run_tests.sh` | PARTIAL: stopped by test time limit; not claimed as passing |
| Physical M-VAVE FM1, OTA handoff, long-duration audio/DMA | NOT TESTED |

## Limitations and release safety

- A successful host build and package validation do not establish that a physical FM1 accepts the flash, that audio never glitches under stress, or that independent flash recovery is guaranteed. Verify on real hardware with a backed-up project and a working independent recovery tool.
- The returning-user Welcome greeting appears on every ordinary boot for configured users; it is a brief read-only preview, not a re-run of first-time format questions.
- Sustain tracking runs in the control thread and derives note length from the audio playhead. If two note events occur entirely between successive UI control polls, they cannot be distinguished; physical timing and dense external MIDI scenarios need measurement.
- SEQ+PRESETS shifting while transport is playing swaps pattern pointers atomically, but audible transition timing under heavy FX load has not been measured on real hardware.
