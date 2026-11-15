# RUPTURE V1.90 — GLIDE / FX8 Verification

Date: 2026-11-15  
Status: **Software RC, physical FM1 verification required**

## Build

- Native target: `--storage --updater --usb-audio-48k` / M-VAVE FM1.
- Jieli cross-compile **PASS**. The 19 audited upstream HAL/boot hashes passed.
- `app.bin`: **392,984 bytes** of **581,564** available. App headroom: **188,580 bytes**.
- Native entry point and ELF/header audit: **PASS**.
- `RUPTURE-V1.90.fwsc`: **610,086 bytes**; verified FM-1_968 package.
- Native app SHA-256: `89907d24aa60303020625d9d90c4c3d02a29b5474b2d04d0459bcb25e6ff9a99`.
- FWSC SHA-256: `6867f5db5babcc4b3e10d27dfccb01e3a90ddafb697967af6f371711eeb87d97`.

## Software tests

The complete `sh tests/run_tests.sh` suite finished with **exit code 0**.
Tests updated for previous legitimate UI/storage evolutions now reflect the actual RUPTURE screens rather than obsolete fixed screen counts or historic source hashes.

- **Eight FX slots:** insertion, duplicate rejection, enable defaults, per-slot bypass, move and remove: PASS.
- **Real DSP order:** COMP → MOSFET versus MOSFET → COMP produces different processed PCM: PASS.
- **Stereo preservation:** Delayed left/right returns remain decorrelated with a subsequent MOSFET slot: PASS. Mid-channel TILT, FUZZ and SAND processing applies the transfer delta to both channels rather than collapsing earlier stereo effects.
- **GLIDE:** linear/exponential pitch convergence in both directions with exact arrival: PASS.
- **SEQ SLIDE:** bit flags set and cleared correctly for individual steps: PASS.
- **PJT35:** FX chain, glide per preset and 256 step-slide flags round-trip; 106-byte versioned extension: PASS.
- **PJT34:** previous project record decoded without invalid state; legacy FX signal flow remains active until the user edits the chain: PASS.
- **Bit-exact V1.89 old FX:** the original V1.89 source file was independently compiled and rendered against the legacy PCM fixture. V1.90 old-mode output has the same SHA-256 `b00a7be00fce2b491413438b11fff58f39c8bf1efaec9583dd0c76d6bbcd7d75`: PASS.
- **OTA/container:** app decrypts to the cross-compiled binary, trusted loader and protected bootheader match V1.89 byte-for-byte: PASS.
- **Web MIDI:** installer HTML and `index.html` both embed the exact V1.90 package, with the prior connection/OTA JavaScript untouched: PASS.
- **Inherited behavior:** 20-frame ENV, BEND graphics, startup POPFIX, flash ID recognition for both Puya `0x856014` and UC25HQ80 `0xB36014`, legacy projects and native OTA unit tests: PASS.

## Controls

FX slots: `SELECT` scrolls all eight slots; `SEL` on an empty slot opens the FX chooser; `K1` opens it for the selected slot; `SEL` on a filled slot toggles on/off; `EDIT` opens the effect's four-knob controls; `ALGORITHM` moves the selected slot and its ON/OFF state in the actual signal chain. Duplicate processor types are intentionally blocked because each native algorithm has a single fixed DSP state/buffer.

GLIDE: `HOME` from SUB OSC 2, then `K1` time 0–2000 ms, `K2` OFF/LEGATO/ALWAYS, `K3` LINEAR/EXP and `K4` envelope retrigger. On the stopped sequencer, choose a step, hold `SEQ` and turn `K4` to set/clear SLIDE.

## Hardware verification still required

The successful software build and host PCM tests do **not** prove: zero FM1 DMA late/guard events under eight simultaneously enabled effects, glitch-free live reordering, hardware-accurate envelope retrigger behavior, flash-save durability after power-cycle, audio quality on the built-in speaker, or actual OTA installation on both FM1 flash batches.

Back up projects, keep the known-working recovery route available, and test at low volume before deploying V1.90 for live use. Firmware downgrade to V1.89 may not understand PJT35 project snapshots. Never disconnect during OTA.
