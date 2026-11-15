# RUPTURE FM1 V1.96 — software validation

Date: 2026-11-15. Release status: **software test candidate; not physically tested on an FM1**.

## STRICT scale guide

- UI order: OFF, GUIDE, STRICT, LOCK.
- On-device saved values: 0, 1, 3, 2. Earlier saved LOCK=2 remains LOCK.
- STRICT ignores out-of-scale physical keys before mono synth/arp processing.
- STRICT masks out-of-scale LEDs on normal performance pages; sequencer step editing and hardware diagnostics retain their navigation indication.
- STRICT ignores out-of-scale USB/TRS MIDI note-on and physical USB MIDI note-on output. MIDI note-offs remain unfiltered.
- STRICT excludes forbidden new presses, count-in prekeys and sustained live-REC captures.
- In-scale stereo PCM is bit-for-bit identical between GUIDE and STRICT in the audio-core regression.
- The oscillator, EQ, GLIDE and effect DSP sample loops were not changed. The strict note mask is calculated per block (for synth and USB MIDI) and only when STRICT is selected.

## Release safety checks

- Cross-compiled app: 402,576 bytes, with 178,988 bytes of application headroom.
- FWSC: 610,086 bytes, verified container and correct flash image decryption.
- Protected bootheader matches V1.95 byte-for-byte.
- OTA loader matches the trusted previous release byte-for-byte.
- Both Puya `0x856014` and UC25HQ80 `0xB36014` flash IDs remain supported.
- Both Web MIDI installers contain the exact generated FWSC; their transport logic is unchanged.
- `bash tests/run_tests.sh`: **PASS** (all included host-side tests, including project persistence, legacy patches, old/new FX audio parity, animations, flash/OTA, and V1.96 hardware-key simulation).

## Before stage use

Physical FM1 validation is still needed. Check out-of-scale key LED darkness on each performance page, held-note transition on a live keyboard, USB/TRS MIDI hardware, live sequencer REC, project save/reboot, and worst-case FX-chain `LATE/GUARD` under extended playback. This host test cannot establish the FM1's realtime CPU margin or certify glitch-free live performance.

## Clean project packaging

Only `RUPTURE-V1.96.fwsc` appears at the project root. The two binaries required for reproducible trusted firmware packaging are under `reference/firmware/`; older test reports are under `docs/history/`. Historical source, artwork, codec fixtures and active regression tests are retained. Temporary build objects and Python bytecode are omitted from the release ZIP.
