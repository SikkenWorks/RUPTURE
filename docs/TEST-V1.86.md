# RUPTURE V1.86 — BEND release validation

## Included
- New **BEND** page opened by holding FX for 700 ms; a regular FX tap still opens the existing FX screen.
- **K1:** momentary rising string bend, releases quickly after turning stops.
- **K2:** selectable 1/4, 1/2, FULL, 1 1/2, DOUBLE; maximum **2 semitones**.
- The pitch factor is applied to the monophonic oscillator path **before** FX. Effects, reverbs and suboscillators retain their existing processing routes.
- Native 240×240 1-bit screen displays range tiles, current semitone offset, bend meter, IDLE/BENDING/RETURN state, with **eight offline-generated 224×30 1-bit mountain sprite frames**; no runtime mountain drawing.
- Native RUPTURE SYSTEM INFO firmware label updated to **V1.86**. No flash/storage format change.

## Verified in software
- **PASS:** JieLi/pi32v2 native FM1 build (`--storage --updater --usb-audio-48k`). App = **357,456 bytes**, application slot = 581,564 bytes, headroom **224,108 bytes**. Startup address `0x02000120` and executable audit passed.
- **PASS:** independent repeated native build produced the same app SHA-256 (`69248ddc21b502172ee1f5056b77184024509a9c1a3502bb44ea57ea8eac33c5`).
- **PASS:** actual audio-engine host test, both CXS and SAW. Neutral BEND PCM matches legacy/uninitialized neutral exactly, full 2-semitone bend changes PCM while keeping **one continuous held voice**, and reset to neutral retains the same note identity.
- **PASS:** 8 PNGs were compared bit-for-bit against ROM-packed bytes. Sixteen 240×240 CRT/VCR ON/OFF BEND renders and both contact sheets were checked.
- **PASS:** 16→4→16 sequencer audio regression and 120,000-state panel-MIDI parity; MIDI burst queue test.
- **PASS:** Puya P25Q80 `0x856014` and UC25HQ80 `0xB36014` flash-ID policy host test.
- **PASS:** release `.fwsc` validated. **Protected 0x4000-byte bootheader and complete trusted OTA loader are byte-identical to V1.85.2**.
- **PASS:** manifest app/firmware sizes and SHA-256 match; WebInstaller and GitHub Pages `index.html` embed precisely the same firmware as standalone `.fwsc`.

## Not yet verified on physical FM1
- Actual FX long-press and K1 encoder gesture feel, animation redraw speed with lots of MIDI notes, live return timing, and real DMA LATE/GUARD under stress.
- V1.86 cold boot/project SAVE/OTA on a recent-chip `0xB36014` FM1.
- Whether the hard speaker plop after prior USB-C use is resolved. This release does **not** modify the PC6 speaker-enable code.

**Installation caution:** back up all user projects and retain a proven recovery route. Start sound tests at low volume. Software validation is not proof of safe operation on every FM1 hardware revision.
