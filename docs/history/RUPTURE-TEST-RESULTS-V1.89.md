# RUPTURE V1.89 – Speaker POPFIX Integration and OTA Audit

Date: 2026-11-13

## Baseline and provenance

- Base: complete **RUPTURE V1.88.3** source, including the corrected 20-frame ENV eruption, eight smoke-settling frames and reverse animation on note release.
- Speaker-plop reference: user-supplied **`RUPTURE-FM1-V1.88.2-POPFIX-SOURCE.zip`**.
- Compared this ZIP against the original V1.88.2: it changed exactly **three native firmware files** (`bringup.c`, `hal/fm1_audio.h`, `native_output_startup.h`) and added eight POPFIX test/host-stub files. No DSP, MIDI, flash, OTA, or graphic files changed in the source reference.
- Transplanted the three POPFIX behaviors into V1.88.3. Verified all three files have **identical executable/preprocessor code to the specific supplied POPFIX version**, after disregarding comments and whitespace. Added the POPFIX host tests to the full source.

## What the POPFIX changes

| Phase | Previous behavior | V1.89 behavior |
|---|---|---|
| Codec startup | PC0, PC2 and PC1 asserted before DMA/I2S startup | Zero-filled DMA/I2S starts first; then PC0, PC2 and PC1 are asserted |
| Codec line timing | Lines enabled in immediate succession | 2 ms after PC0, then PC2/PC1; another 5 ms settle |
| First LED activation | Immediately preceding audio startup | After audio startup and a watchdog-fed **300 ms** quiet interval |
| PC6 analog enable | Deferred until 400 ms stable LED bank, USB and DMA/clock conditions | **Unchanged protection**: same 400 ms / USB / DMA/clock gates |
| Audible PCM | Existing delayed/muted-to-audible ramp | **Unchanged** |
| Warm audio restart | Original codec init sequence | Detects already asserted control lines to avoid an extra control edge |
| Diagnostic A/B | No ordering switch | `FM1_AUDIO_LEGACY_CODEC_ORDER` default `0`; `1` restores former order for diagnostic builds |

This timing separation is a targeted **mitigation** of the startup pop. Software-only tests cannot establish that an analogue amplifier/bias transient is acoustically eliminated.

## Build and integrity

Build flags: `--storage --updater --usb-audio-48k`

| Check | Result |
|---|---|
| Native FM1 cross-compile | **PASS** |
| Native app (V1.89) | 386,368 bytes |
| Previous V1.88.3 native app | 386,248 bytes |
| App size increase | 120 bytes |
| Available app headroom | 195,196 bytes |
| `.fwsc` package | 610,086 bytes |
| App SHA-256 | `5789656a28746cac09428444098e7d029ae2ed813927eced5db2f720e6d18f80` |
| FWSC SHA-256 | `4255049edb3003c1c0868e075a8c0a226aa8c33af838aa8cf447cac6c103d74b` |

## OTA and regression guarantees

- **PASS:** Trusted `ota.bin` is byte-identical to V1.88.3. SHA-256: `d71d1b7c12adede951605e758a3580dacccedc427d49e1fceaf51decccc68a4b`.
- **PASS:** Protected first 16 KiB of `flash.bin` is byte-identical to V1.88.3. SHA-256: `a65e0e1a2fad1db943c00b529b45553c524257146145db61b69628cefcf2b2c9`.
- **PASS:** Native app decrypts back exactly from the rebuilt firmware's JieLi SFC-XOR app area; FWSC structural/CRC verification passes.
- **PASS:** Current WebInstaller and GitHub Pages contain the exact V1.89 FWSC with the matching package SHA and byte count.
- **PASS:** The OTA transfer, Web MIDI generic Windows port probing, NUL-safe identity decoding and confirmation logic are identical to V1.88.3 except version label, package hash and length.
- **PASS:** Both Puya P25Q80 (`0x856014`) and UC25HQ80 (`0xB36014`) flash chips are recognized; no chip-detection code was edited.
- **PASS:** All native V1.88.3 ENV/hold-frame sprites, animation state machine and BEND frames are unchanged. The audio engine, USB, MIDI, DSP, effects and flash storage are unchanged.

## Executed tests

- **PASS:** Supplied `tests/pop_fix/run_pop_tests.sh`: codec pins after DMA, PC6 low, warm-restart edge guard, legacy A/B mode, USB-qualified PC6 delay, analog settle, LED gate and audio ramp.
- **PASS:** New `tests/test_v189_popfix_ota.py`: exact imported POPFIX code semantics, inherited V1.88.3 source fingerprints, FWSC/header/OTA identity, Web MIDI unchanged.
- **PASS:** V1.85 real-time panel MIDI / queue tests and mono instrument sanity checks.
- **PASS:** V1.85.2 dual JEDEC chip tests and V1.86 MIDI connection regression.
- **PASS:** V1.56, V1.57, V1.58, V1.66, V1.70, V1.88.1 and V1.88.3 animation tests.
- **PASS:** 60/60 panel sprite PNG-to-ROM comparisons; eight settling ENV and eight LFO hold-sprite comparisons.
- **PASS:** `tools/build_site.py`, `tools/test_site.py` and release website embed checks.

The historical catch-all `tests/run_tests.sh` is known to halt at the unrelated obsolete V1.27 settings-screen pixel coordinate assertion (line 19 of `test_v127_native_ui.c`); this already occurred in V1.88.3. The targeted tests above passed. Do **not** claim the entire legacy suite passed.

## Hardware validation required

**No physical M-VAVE FM1 speaker or OTA transfer was tested in this execution.** The original POPFIX behavior has been faithfully integrated and compiled, but whether it audibly eliminates the pop on cold boot, after warm restart, on external audio connections or across both FM1 flash variants must be tested on the device. The new startup sequence also warrants verifying visible LED timing, audio latency, recovery-button paths, normal playback and post-OTA reboot. Before installing, back up projects and keep a verified FM1 recovery route available.

**Status: software release candidate, pending physical FM1 verification.**
