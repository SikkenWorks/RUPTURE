# RUPTURE V1.77 — build and verification report

**Status:** JieLi pi32v2 build successful; host audio, router, website and OTA integrity checks passed. **Not tested on the physical FM1.** No full historical regression pass is claimed.

## Source and build

- Baseline: `RUPTURE-FM1-V1.76-FULL-SOURCE-FWSC.zip`, with all native sources and existing functions retained.
- Cross-compile: `python3 vendor/etch-v1.88-source/firmware/fm1-native/build.py --storage --updater --usb-audio-48k` using JieLi pi32v2 clang. Result: valid native application, 345,884 B, 235,680 B free under the 581,564 B application limit.
- Built `.fwsc`: 610,086 bytes; SHA-256: `14e66bc04793f72c95be576ef049b4e0b81d25ca1034b4ea297955324cf09b4d`.
- Firmware identity: `FM-1_968` retained.
- `ota.c` and `audio_core.c`: byte-for-byte identical to V1.76. Native bootheader first 16 KiB, trusted `ota.bin`, and non-app flash bytes identical to the original V1.71 recovery reference.
- No storage codec, project or sequence format migration.

## Targeted tests

- `tests/test_v177_navigation.py`: PASS. Unused SELECT falls through to the exact PLAY screen list, while existing SELECT handlers and protected flows remain separate. This is a source/router contract check, not a physical encoder test.
- `tests/test_v177_sound.c`: PASS. Uses actual native SAW renderer and `native_fx.c`.
- `tests/test_v148_saw_alias.c`: PASS. 7 band-limited mip bands and alias guard remain bounded.
- `tests/test_v09_fx.c`: PASS. Chorus, delay, reverb and EQ active independently.
- `tests/test_v122_fx_quality.c`: PASS. TILT, SANDPAPER, FUZZ, dry levels and post-mix regression.
- WebInstaller and site tests: PASS. Embedded `.fwsc`, SHA-256 and README/CHANGELOG fallback bytes equal the release files. OTA installer transfer code copied unchanged from the trusted baseline.
- Native UI screen frames re-exported with the current source; HELP card additions fit the existing 13-card layout.

## Measured host audio changes

A 65 Hz bandlimited SAW test (44.1 kHz; no additional oscillator/filter): new/old derivative energy **0.74** and RMS ratio **1.00** under the factory SAW macro values. No samples clipped in this test. This is an objective indication of lower upper-harmonic emphasis, **not** proof of analog/Taurus sound quality.

The post-mix reverb was measured with a 110 Hz burst, then a silent tail. Preset ratios and tail RMS are for this **one** test waveform, not loudness targets or hardware measurements:

| Reverb preset | 100% wet RMS / dry RMS | Audible tail RMS |
|---|---:|---:|
| SPRING | 0.99 | 1212 |
| HALL | 0.66 | 2563 |
| ROOM | 1.09 | 1840 |

100% MIX remains completely wet without a dry feed; a wet-only reverb can legitimately have different RMS from the dry oscillator. Crucially, the actual delayed, decaying tail has nonzero energy and the three presets produce different PCM. Wet return level is boosted relative to V1.76. All presets use the same fixed 4096-sample tank, the same five return taps, unchanged global FX allocation, no extra per-voice processing and no additional audio buffer. Reverb coefficient/tap timing calculations occur only when parameters change. Actual FM1 worst-case DMA cycles were **not measured**; no claim of equal hardware CPU utilization is made.

## Remaining limitations

- A historical UI test (`tests/test_v127_native_ui.c`) failed an old hard-coded pixel assertion at line 19 (`count_pixels(166,110,14,14)>0`). The comprehensive suite stopped there. This failure has not been established as pre-existing and needs separate triage. It does not serve as a direct test of the new SELECT fallback.
- The firmware is not proven stable/quiet on the physical FM1. Listen to SAW at low/high notes and verify spring/hall/room at 0/50/100% MIX, with other FX bypassed.
- Hardware CPU/DMA budget under maximal FX and USB load is not verified. The new code does not allocate additional buffers, FX processors, oscillators, or audio voices; this is a structural constraint, not a hardware benchmark.
- Verify SELECT navigation on FILTER, ENV, SOUND SHAPE and SUB as well as preservation of SELECT navigation in SEQ, FX, HELP, settings, storage and all SAFE/OTA states.
- Make a project backup and retain the proven Pico recovery before any installation.
