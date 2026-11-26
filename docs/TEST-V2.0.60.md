# V2.0.60 validation

Software candidate. Physical FM1 audio, power-off and long-duration reliability remain unverified.

- STABLE and CORE2 native pi32v2 builds pass their entry, SRAM/link and 581564-byte app allocation audits.
- `python3 tests/run_v260_checks.py` runs the new production-function storage handoff test and 18 existing PCM/MIDI/CORE2/flash/USB/startup regression programs.
- The new test executes the actual foreground pause/start functions and all five store callbacks against a simulated DMA/HAL. It checks all 65536 int16 amplitudes, 10000 warm transactions, distinct zero DMA halves, nested calls, grouped catalog reads, failed reads, timer wrap, absent ISR, rejected zero publications, explicit fault rearm policy and unchanged cold-boot gating. ASan/UBSan are enabled. Leak detection is disabled in this runtime because it runs under ptrace; the harness performs no heap allocation.
- Original CORE2 tests compare 100 ordered effect pairs and six heavy chains across 81408 stereo frames, plus 384000 frames with worker stalls/partial-completion injections. Guard tests exercise one million forged replies and one million pressure decisions.
- Storage regressions include A/B fallback and torn-write behavior, preferences, expression profiles, flash bounds and legacy wipe. USB regressions include preference persistence, descriptor shape, packed PCM and resampler behavior.
- `python3 tests/test_v260_release.py` verifies both complete FWSC containers, their exact app payloads, matching offline installer embeds and hashes, original OTA/header bytes, and pinned preservation of synthesis/FX/store/HAL sources.
- The historical `tests/run_tests.sh` starts with V2.0.58-only release assertions and is retained for provenance. Use the current runner above for this candidate; the entire historical suite has not been requalified.

For diagnosis, code changes and the outstanding physical acceptance procedure, see [AUDIO-OUTPUT-AUDIT-V2.0.60.md](AUDIO-OUTPUT-AUDIT-V2.0.60.md).
