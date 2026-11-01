# RUPTURE V1.34 — Engineering validation

## Changes

- Four-band 44.1 kHz fixed-point Channel EQ 2.0 (LOW shelf, BODY bell, BITE bell, AIR shelf) in the global post-mix signal path.
- ±9 dB, 0.5 dB gain steps, independently variable frequency, BODY and BITE Q (0.3–8.0), native 240×240 response graph using the actual coefficients.
- Three EDIT pages: GAIN, FREQUENCY, PEAK WIDTH (Q), selected with repeated EDIT presses; ALGORITHM explicitly switches to/from LEGACY EQ.
- Coefficients computed outside the audio IRQ and smoothed on the sample path; positive boost headroom compensated.
- New projects default to EQ2. PJT34 saves EQ2 mode, four frequency controls and two Q settings; decoding PJT33 and earlier defaults to legacy EQ for existing-project sound parity.

## Tests performed

| Check | Outcome |
|---|---|
| JieLi FM1 `--storage --updater --usb-audio-48k` crossbuild | PASS; 226,320-byte app; 355,244-byte application headroom |
| Existing native regression suite `bash tests/run_tests.sh` | PASS (exit code 0); includes UI, storage, MIDI, FX, legacy compatibility, V1.33 sequence save/load |
| Native EQ2 coefficient tests | PASS: neutral PCM exact, two bell Q widths, shelves, peak center gain |
| Native FX integration test | PASS: neutral PCM parity, Q-dependent response, auto headroom |
| PJT34 codec and legacy compatibility | PASS: new records, old records default legacy, corruption/truncation rejection |
| Native 240×240 EQ page screenshots | EXPORTED; GAIN, FREQUENCY, PEAK WIDTH |
| Firmware container and OTA | VERIFIED; same protected 16 KiB bootheader and OTA loader as preserved reference |
| Firmware SHA/length inside WebInstaller and index.html | VERIFIED byte-identical |
| GitHub Pages/site tests | PASS |
| Physical FM1 audio and CPU under extreme FX load | NOT YET PERFORMED |
| Physical save/load and flashing of V1.34 | NOT YET PERFORMED |

## Known limitations / precautions

- Host tests and a crossbuild do not establish physical sound quality, absence of zipper noise on actual FM1 hardware, worst-case DMA deadline or recovery safety. Back up projects, preserve known-good prior firmware and verify the original recovery route before flashing.
- The source firmware's protected OTA loader remains unchanged; its flash display progress remains managed separately from the application.
- Older projects intentionally remain in LEGACY EQ mode until the user selects EQ2 and saves, to avoid unrequested tone changes.
- EQ2 is post-mix and monophonic; its coefficient preparation is not per-voice and adds runtime biquad work. This is not a zero-CPU feature.
- The Q control applies to the BODY and BITE bell filters. LOW and AIR are shelf filters and expose frequency, not Q, in this release.
