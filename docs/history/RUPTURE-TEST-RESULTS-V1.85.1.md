# RUPTURE V1.85.1 — Sequencer LENGTH=4 hotfix test results

## Root cause

The SEQ UI in `bringup.c` accepted lengths 4, 8, 16, 32, 64, 128, but the native audio step scheduler (`audio_core.c`, `perf_tick`) only accepted 8, 16, 32, 64, 128, 256. Any `c->seq_length==4` was silently replaced with 16 **only in the audio engine**. The LCD renderer separately displayed the correct 4-step playhead, so playback appeared to wrap while audible note triggers continued across sixteen steps.

## Code changes: exactly one FM1 source file

`vendor/etch-v1.88-source/firmware/fm1-native/audio_core.c`:

1. Allow sequencer length **4** in the same validation as the other lengths.
2. Wrap the next *audio* step to the active length before reading `pattern->steps[step]`, protecting live 16→4 shortening even if the pending DSP step index is >=4.

No other firmware C, H or assembly source differs from V1.85. No notes are deleted on shortening; the sequencer pattern contents, sequencer save/load, UI, reverb, synth voices, filter, USB, MIDI, LED/PC6, first-boot, project data, bootheader and OTA have not been modified.

## Executed checks

- **PASS** Host DSP integration `sh tests/test_v1851_length_audio.sh`: exercises `etch_native_render_stereo` directly, confirming 16-note sequence → live length 4 → return to 16; tests that audible/pitched steps and playhead are inside the length; steps 5–16 are preserved; fresh four-step loop; unchanged lengths 8, 16, 32, 64, 128.
- **PASS** 16-step heavy SUB/FX MIDI/sequence audio parity: `PCM_HASH 4f603683ade7da59` from both V1.85 and V1.85.1 using `bench_v181_note_bursts.c`; zero difference in unrelated audio output.
- **PASS** Existing V1.85 panel MIDI test (120,000 randomized state transitions) and exceptional MIDI queue stress.
- **PASS** Native FM1 JieLi build with persistent projects, native OTA and 48-kHz USB audio; a repeated build is SHA-256 identical.
- **PASS** Strict hash audit over 177 V1.85 FM1 C/H/S files: only `audio_core.c` differs; V1.85 source baseline hashes provided in `reference/`.
- **PASS** Firmware package validation; 0x4000-byte protected bootheader and trusted OTA loader byte-for-byte identical to V1.85.
- **PASS** WebInstaller/standalone FWSC/website exact embedded package match, SHA-256 and byte count.

## Build and package identifiers

- Application `app.bin`: **348,380 bytes**; SHA-256 `f625d120a5c2008af40683db0221ec7e0933e7686899061fd876d4134fb16f04`.
- Application slot: 581,564 bytes; free **233,184 bytes**.
- `RUPTURE-V1.85.1.fwsc`: **610,086 bytes**; SHA-256 `9acc914d65bc37448ced04ece12f396c8748c2ccc9e7310a69e3a5712fdb3f7f`.
- Product identity: `FM-1_968`.
- Build: `JIELI_TOOLCHAIN=<toolchain> python3 vendor/etch-v1.88-source/firmware/fm1-native/build.py --storage --updater --usb-audio-48k`
- Package: `python3 tools/package_v1851_release.py` (using preserved V1.85 OTA/reference assets), then `python3 tools/build_site.py && python3 tools/test_site.py`.

## Limitations and version display

The M-VAVE FM1 itself has **not** been flashed or tested in this environment. Playback after a real OTA and live K1 encoder edits still needs checking. The system-info UI source is intentionally untouched, honoring the user's "do not change anything else" requirement; therefore the device's pre-existing on-screen version label may still show `V1.85` even though the installation bundle/manifest is `V1.85.1`. The functional distinction is the corrected four-step playback.

Make a project backup and keep an existing hardware recovery route available before flashing.
