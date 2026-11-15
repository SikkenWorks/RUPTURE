# RUPTURE V1.88.1 — Approved poster-frame correction

Date: 11 November 2026

## Why this fix exists

RUPTURE V1.88 **did not contain the five illustrated 8-frame animations from the approved poster**. Instead, it shipped substitute, hand-drawn graphics based on the themes. This was a mismatch with the user's approved design.

V1.88.1 uses the **exact 40 distinct illustrated poster cells as its graphical sources**. The source is a 1536 × 1024 JPEG composite supplied by the user, **not individual high-resolution PNG sprite source files**. The source cells must be cropped, resampled to the existing 224 × 30 pixel BEND viewport and thresholded to native 1-bit. The detail lost at 30 pixels high cannot be fully recovered. The original JPEG, all 40 individual crops, 40 converted PNGs, source bounds and reproducible conversion script are included in the full source.

## Five K3 mapping and playback

- LINEAR: the **approved poster's Everest frames**, replacing the previous V1.87-derived animation.
- SOFT: the approved poster's monolith (including its carved circle emblem).
- HARD: the approved poster's pirate flagship erupting from a field.
- S-CURVE: the approved poster's close-up from ship hull to the mast and pirate flag.
- EXP: the approved poster's Kraken sequence.

The four eruption stories use frames 0–4 while the bend rises, frames 5–7 as it returns, and frame 0 when idle. This avoids erroneously displaying the collapsed end-of-storyboard frame at maximum bend. The S-CURVE camera uses frames 0–7 as pitch rises, reversing when pitch drops. All four eruption animations keep the preexisting UI-only shake; the close-up does not shake.

## Graphics validation

- **PASS:** source poster SHA-256 `a3f13d341145dc9e4647f40868293cf638465f59dfcb4294ca589505bca80387`.
- **PASS:** all 40 native 224 × 30 monochrome PNG sprites are deterministically derived from the actual poster cells, not procedural recreations.
- **PASS:** all 40 PNGs have bit-exact parity with the compiled ROM header (`5 × 8 × 30 × 28 = 33,600 bytes`).
- **PASS:** 40 out of 40 newly converted PNGs differ from the original V1.88 substitute graphics.
- **PASS:** source crops and final 240 × 240 native UI screenshots are included for inspection; 80 screens rendered for five curves × eight frames × CRT on/off.
- **PASS:** shake changes only four eruption frames; close-up stays stable across shake phases.
- **PASS:** host compiled storyboard test checks full bend displays the explosive climax and release displays poster collapse frames.

## FM1 native build and package

- **PASS:** cross-compiled for JieLi/PI32V2 with `--storage --updater --usb-audio-48k`.
- **PASS:** two deterministic builds of the identical native application.
- Native app **386,156 bytes**, free application headroom **195,408 bytes**.
- App SHA-256 `060a973e623d279ee7279c71f3dc3aec3902d58086fa4f6c4f4eb514c99503b9`.
- Firmware `.fwsc`: **610,086 bytes**, product ID `FM-1_968`.
- Firmware SHA-256 `348d9ac74618b8c9cfaea60875d95ae6320a189b14a0a66076805b861c7db9ba`.
- Trusted OTA loader **byte-identical to V1.88**, SHA-256 `d71d1b7c12adede951605e758a3580dacccedc427d49e1fceaf51decccc68a4b`.
- Protected first 16 KiB flash header **byte-identical to V1.88**, SHA-256 `a65e0e1a2fad1db943c00b529b45553c524257146145db61b69628cefcf2b2c9`.
- **PASS:** WebInstaller and website embed this exact FWSC, with matching hash/size and original V1.86 NUL-safe identity parser and read-only generic MIDI discovery.

## Regressions

- **PASS:** sixteen critical CXS, SAW, audio/DMA, FX, MIDI, storage, flash, OTA and hardware ISR source files are byte-identical to original V1.88.
- **PASS:** five bend curves generate non-retriggering sustained CXS/SAW PCM and return to neutral (`test_v1861_curve_pcm.sh`).
- **PASS:** native BEND CXS/SAW PCM regression (`test_v186_bend_audio.sh`).
- **PASS:** sequencer 16→4→16 DSP regression (`test_v1851_length_audio.sh`).
- **PASS:** Puya and UC25HQ80 JEDEC support (host `test_v1852_flash_chip.c`).
- **PASS:** Web MIDI NUL-safe identity and generic Windows MIDI port detection (`test_v186_connection_fix.mjs`).
- **PASS:** native site/manifest/installer parity (`tools/test_site.py`).
- **PASS:** new V1.88.1 poster provenance, packed ROM parity, source audit and signed firmware container (`tests/test_v1881_approved_poster.py`).

The historical `tests/test_v1852_flash_chip.py` contains a V1.85.1-specific source-diff assertion and is not a version-independent regression test; that specific historical assertion fails when run against V1.88.1 because the synthesis engine has legitimately evolved since V1.85.1. The underlying dual-flash host C test and V1.88.1 source parity tests pass.

## Performance and hardware caveats

The new poster sprites use the **same 33,600-byte ROM allocation as V1.88**, with no additional image-decoding, rendering, particle system, filter or memory allocation in the audio ISR. The existing UI-level frame choice adds a small constant-time check to follow the poster's rise/return sequence. LCD rendering remains subject to the preexisting audio-deadline priority gate. As with V1.88, the shake may increase UI display activity while bending, but the DSP and audio ISR source code are unchanged.

**NOT TESTED:** real FM1 installation or boot, real LCD throughput/DMA slack, audible speaker plop, actual UC25HQ80 flash save/OTA, and real-hardware USB reconnection. Test at low volume, back up projects, and keep a verified FM1 recovery path available.
