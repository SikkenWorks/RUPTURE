# RUPTURE V1.75 — Build and test report

## Scope

Source base: RUPTURE V1.74 full-source project. Target: M-VAVE FM1 (JieLi pi32v2). Host-side builds and tests only; **physical FM1 has not been tested**.

## Implemented behavior

- REC pressed when playback is stopped arms recording only; it does not start playback or alter any sequence notes.
- PLAY pressed after REC starts exactly eight quarter-note metronome beats (two 4/4 bars). Sequencer and REC start together at the next quarter-note boundary, step 1. The audio render callback itself asserts step 1 at that sample boundary, not a delayed UI timer.
- REC pressed while the sequencer already plays starts/stops live recording immediately without counting in.
- REC or PLAY while count-in runs cancels the countdown, with no recorded notes.
- Playing keys during armed/count-in does not accidentally insert/delete steps. The physical synth remains playable.
- The existing independent PCM click is reused. Bar 1 and 2 beat-one accents remain audible; no extra synth voices/FX buffers are created.
- Sequencer panel displays ARM and COUNT plus a two-bar beat indicator. Its grid boundary and physical SEQ LED pulse with the countdown beats. UI effects run outside the audio interrupt.
- PRESETS stays BPM; ALGORITHM controls click volume while REC is armed and during live recording.
- 43-page Quick Tour, 13 HELP cards, README, cheat sheet, installer and website updated. SYSTEM INFO firmware label is now V1.75.

## Builds and integrity

| Check | Result |
|---|---|
| Native JieLi pi32v2 crossbuild (storage/updater/UAC48/multi-project) | PASS |
| Repeated native binary build | PASS, byte-identical SHA-256 |
| Native application size | 344,728 bytes of 581,564; 236,836 bytes remain |
| Firmware package size | 610,086 bytes |
| Firmware identity | `FM-1_968` preserved |
| Application SHA-256 | `c8a7ebb464d512c113951dd5ce92dd4afdc566c33dc82602def4e80f63eb0ad0` |
| Firmware package SHA-256 | `45a01210c6ebad57ab970689b9fe9c76a558ed7a66abfe154255446e9e4c6fa2` |
| Protected first 16 KiB of flash | Exact V1.71 reference equality checked |
| Embedded OTA loader | Exact V1.71 reference equality checked |
| Native OTA C source | Byte-identical to V1.74 |
| Native project codec | Byte-identical to V1.74 |
| WebInstaller embedded firmware | Exact .fwsc byte equality confirmed |
| GitHub Pages firmware | Exact .fwsc byte equality confirmed |

## Executed host tests

1. `test_v175_countin.c` — exact eight-beat count-in at 30, 60, 120 and 240 BPM, beat accents, abort, change of BPM between beats, sample-accurate step-1 handoff: **PASS**.
2. `test_v175_record_ui.c` — native 240 × 240 1-bit ARM, bar 1 / bar 2, pulse on/off and REC states: **PASS**.
3. `test_v175_transport_contract.py` — REC/PLAY source routing, no accidental edits during armed/count-in, metronome mapping, boot/OTA/storage source invariants: **PASS**.
4. `test_v174_seq_shift_sustain.c` — existing sustain capture, lossless wrap shift, HOLD references/locks: **PASS**.
5. `test_v172_metro_and_step.c` — independent click and complete step-clear, all HOLD and lock references: **PASS**.
6. `test_v123_help_cards.c` — all 13 HELP cards: **PASS**.
7. `test_v171_tutorial.c` — all 43 pages and optional tutorial workflow: **PASS**.
8. `test_v174_welcome.c` — existing returning-user welcome preview: **PASS**.
9. `test_v173_mapping_docs.py` and `test_website_release.py`: **PASS**.

The complete historical stress/regression suite was not rerun. **Host validation is not a physical recovery or audio reliability guarantee.** Before applying to a real FM1, back up all projects and preserve a known-working external recovery route.
