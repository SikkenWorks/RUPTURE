# RUPTURE

**Cycle Exchange Synthesis (CXS)**  
A monophonic bass synthesizer for the M-VAVE FM1.  
**SIKKENWORKS | Born out of passion.**

RUPTURE is a playable, programmable bass instrument created for a hands-on workflow on the FM1. Its custom CXS synthesis engine, two independent suboscillators, modulation, filter and effects are controlled directly from the hardware.

## Synthesis and controls

- **PLAY:** K1 PRESSURE, K2 BALANCE, K3 MEMORY, K4 DENSITY control the main CXS sound.
- **HOME:** a short press cycles through PLAY, SOUND SHAPE, SUB OSC 1 and SUB OSC 2. A long press returns to PLAY.
- **SOUND SHAPE:** K1 WEIGHT, K2 EDGE, K3 BITE, K4 PUNCH.
- **SUB OSC 1 / SUB OSC 2:** independent TYPE (OFF/CXS/CLASSIC), OCTAVE, DETUNE and DRIFT controls. ALGORITHM adjusts each SUB's BLEND. CXS and CLASSIC feed the existing audio engine; the CLASSIC route bypasses the main CXS filter while retaining envelope and effects.
- **FILTER:** resonant filtering and tone shaping. **ENVELOPE:** ADSR. **LFO:** modulation of the CXS synthesis engine. **EDIT:** Channel EQ 2.0 with LOW shelf (80 Hz), BODY bell (250 Hz), BITE bell (1.5 kHz), AIR shelf (5 kHz). Gains are ±9 dB in 0.5 dB steps. Each band has an adjustable frequency; BODY/BITE have Q 0.3–8.0 to change peak width.
- **NOTE MEMORY:** six-note monophonic memory, LAST/HIGH/LOW priority, LEGATO, GLIDE, pitch bend and octave shifting using OCT−/OCT+.
- **FX:** REVERB, DELAY, CHORUS, MOSFET, COMPRESSOR, TILT EQ, OCTAVE FUZZ and SANDPAPER. Use SELECT to choose an effect, SEL to enable or bypass it, and K1–K4 to change its parameters.

**Channel EQ 2.0:** Press EDIT. The initial GAIN page maps K1 LOW, K2 BODY, K3 BITE and K4 AIR. Press EDIT again to cycle GAIN → FREQUENCY → PEAK WIDTH (Q) → GAIN. SEL does not switch EQ pages. On FREQUENCY, K1–K4 choose the center or shelf frequencies of the four bands. On PEAK WIDTH, K2 sets BODY Q and K3 sets BITE Q; higher Q gives a narrower peak and lower Q a wider one. The response graph is calculated from the actual filter coefficients. ALGORITHM explicitly switches between EQ2 and LEGACY EQ; older saved projects stay on LEGACY until changed. New projects use EQ2. SAVE stores selected EQ version, gains, frequencies and Q in PJT34 while preserving older project formats. Changes are smoothed, EQ2 uses global post-mix filtering, and a signal-dependent peak guard intervenes only near clipping; EQ boosts are not offset by a global volume cut.

**Ladder performance and safety:** FILTER → LADDER selects the four-pole resonant low-pass with a nominal 24 dB/octave slope. The ladder has an optimized, bounded Q15 signal path to reduce audio-interrupt work. AUDIO DIAGNOSTICS exposes LATE and GUARD measurements for hardware diagnosis. When the FM1 detects repeated missed audio DMA deadlines it deliberately mutes audio and shows AUDIO STOP / DMA TIMING FAULT, while true boot/storage recovery remains SAFE MODE. Do not mistake either warning for a successful performance run. Confirm operation under maximum RESONANCE, CUTOFF, DRIVE and FX on actual hardware, and keep a working recovery route.

**Navigation:** SELECT changes the selection or current step. SEL opens or confirms an action. K1–K4 edit the functions shown on screen. ALGORITHM works only where it has an explicit assignment, including editing names, extending sequencer sustain, scrolling HELP and changing SUB BLEND. Elsewhere ALGORITHM is inactive rather than duplicating a parameter knob.

## Sequencer and arpeggiator

The built-in step sequencer supports lengths of **4, 8, 16, 32, 64 or 128 steps**. Unused steps are hidden from its grid at lengths 4 and 8. SELECT chooses a step, playing a key toggles notes, K3 changes step velocity and ALGORITHM can extend or shorten a note's sustain into empty steps. REC toggles live sequence recording and lights its dedicated LED only while recording is enabled. When recording is off, its LED remains unlit. PLAY/STOP controls the transport. Long SEQ opens the independent sequence library.

The arpeggiator offers several direction and riff modes, with rate, octave range and gate controls. Scale Guide and Keylock are available for note selection.

## Save, projects and sequences

RUPTURE offers **12 named project slots** and **8 sequence slots per project**, subject to flash capacity. Use SAVE on a HOME page to open PROJECTS or SAVE in SEQ to open the sequence slots. Navigate with SELECT:

- On an empty slot, **SEL or SAVE** opens the name editor with a blank field, including old factory placeholders `ETCH24`–`ETCH31`. Use SELECT for the cursor, ALGORITHM for characters, and SAVE to store the name **and actual note data**. Do not remove power before the save completes.
- On an occupied slot, **SEL** requests confirmation to open. **SAVE** stores current content in that slot, with overwrite confirmation when it is a different slot from the one currently in use.
- SEL+SAVE on an occupied slot enters the existing rename function. Both libraries display an **ACTIVE** marker beside the currently opened project or sequence, independently of the highlighted row. Saved sequences retain their own LENGTH and BPM, plus note data and step parameters; sequences recorded before the per-sequence metadata marker use the project's stored timing as a legacy fallback. Verify the on-screen SAVE/LOAD confirmation and keep backups of valuable patches.

User data is written through the existing CRC-checked A/B flash journal. Storage is not assumed safe when unidentified data is present. A SAVE FAILED message should never be ignored.

## GLO, SETTINGS and HELP

The GLO menu contains selectable monochrome cards for SETTINGS, HELP, AUDIO DIAGNOSTICS, ABOUT and REPORT A BUG. Choose a card with the **SELECT** encoder and press **SEL** to open.

SETTINGS contains **MASTER VOLUME** (device-wide, 0–200%), **AUTO SAVE**, **AUTO LOAD PROJECT**, **STORAGE** and **OTHER**. SELECT highlights an option. When **MASTER VOLUME** is highlighted, K1 immediately adjusts the percentage. SEL still opens its dedicated volume page, where SELECT or K1 adjusts the value. **AUTO SAVE** toggles directly with SEL on its selected row, without opening another page. **AUTO SAVE is OFF by default**, including existing installations that have not opted in. When enabled, it only runs after an explicit first save/load, when the transport is stopped, no notes are playing and storage is safe. Your choice is retained through a global journal record, independent of project names. **STORAGE** offers **PROJECTS** and **WIPE ALL**. The project catalog is refreshed on entry, even when the normal project browser has not been visited since boot. PROJECTS lists the twelve saved projects, each with OPEN, SEQUENCES, DUPLICATE, RENAME and DELETE actions. SEQUENCES displays the eight stored sequences within the chosen project, with OPEN, DUPLICATE, RENAME and DELETE. To edit sequences from another project, first confirm opening it. Duplicates are written into the first free slot, never overwriting another project or sequence. The final remaining project can now be deleted and a fresh unsaved project created. A project that owns shared waveforms cannot be removed while other projects depend on its wave library; this shows a specific SHARED WAVES warning instead of a misleading SAVE FAILED. After a rejected edit, the firmware checks and restores access to the valid saved catalog. Deleting the final project also resets device-global settings stored in its project banks. Empty legacy ETCH 24–31 factory sequence labels are ignored where no pattern data exists; real stored sequences remain intact.

**AUTO LOAD PROJECT** opens a list of **NEW** and only the actually stored projects. NEW is the default, even when saved projects exist. SELECT chooses a candidate and SEL stores the selection. If no projects exist, only NEW is offered. On startup, a selected saved project restores its sound and sequencer; if that project has been removed, the firmware starts NEW instead. This is independent of AUTO SAVE.

**WIPE ALL** requires a clear YES/NO choice, defaults to NO and is irreversible. It erases all known RUPTURE and legacy firmware user-storage sectors (`0x97000` through `0xDFFFF`) and verifies they are blank through a dedicated bounded erase/readback path. The ordinary SAVE path cannot erase old firmware storage areas. It never erases the bootloader, application, update loader or OTA staging and cannot promise to erase unrecognized, undocumented data elsewhere in flash. Back up your work before confirming.

**OTHER** contains **SCREENSAVER**. Press SEL to open its dedicated settings screen. SELECT chooses ENABLED or START AFTER; SEL toggles ENABLED, while K1 changes the inactivity delay when START AFTER is selected. The default is **ON / 30 seconds**. Available delays are 15, 30, 45, 60, 90, 120, 180 and 300 seconds. Both settings persist across restarts through the device-global journal.

**REPORT A BUG** displays the author-supplied, white-on-black QR image as a crisp 1-bit pixel bitmap. The original source image is included in the project assets and opens the RUPTURE issues page. **ABOUT** is an individually scrollable GLO page containing the complete personal origin story of RUPTURE. Turn SELECT to scroll the text on the FM1 display. Hold GLO for PANIC.

HELP consists of scrollable cards: SELECT changes card and ALGORITHM scrolls the contents. AUDIO DIAGNOSTICS shows read-only measurements of the realtime DMA deadline, FX render time and skipped waveform visuals.

## Screen and performance

The 240×240 1-bit screensaver uses the supplied Utrecht city skyline with the Domtoren and canal-front buildings. After the configured inactivity delay (30 seconds by default), the pixelated foreground street fractures, opening a progressively wider ravine. The skyline remains static while the road breaks apart; the last animation frame holds. The animation runs only in the UI main loop with the existing framebuffer and no additional DSP processing. It is disabled during active notes, sequencer playback, OTA, storage prompts and recovery.

## USB, firmware and safety

Normal USB manufacturer/product names are **SIKKENWORKS / RUPTURE**. The trusted `.fwsc` installer identity is **FM-1_968** to preserve the known OTA compatibility chain. The website's WebInstaller shows the USB firmware-transfer progress. A separate protected OTA loader takes over during flash writes and does not provide an FM1-screen RUPTURE progress bar.

Install only on compatible hardware, back up projects, keep the device connected while the loader is active and preserve your known recovery route. RUPTURE is regularly tested on physical FM1 hardware by its author. New revisions should still be checked for the specific changed behaviors before release. An emulator or successful firmware build cannot prove reliable on-device operation.

## Source code and build

The complete native source, firmware build scripts, OTA packaging scripts, GPL notices, tests, UI renderer and documentation are included in the full-source release. The native sources reside under `vendor/etch-v1.88-source/firmware/fm1-native/`. The `tests/` directory includes host regressions. Build against the supported JieLi pi32v2 toolchain using `build.py --storage --updater --usb-audio-48k --multi-project`.

`index.html` is the unified GitHub Pages website and WebInstaller. `WebInstaller.html` is a standalone version. The README describes the instrument, controls and complete credits; the separate `CHANGELOG.md` contains release history.

## Credits and licensing

- **Joppe Sikken / SIKKENWORKS:** RUPTURE product concept, sound design and Cycle Exchange Synthesis, monochrome FM1 interface, integration, website, project development. [SIKKENWORKS](https://github.com/SikkenWorks).
- **Leo Kuroshita (@kurogedelic) / Hügelton Instruments:** [Felucca](https://github.com/hugelton/Felucca) custom FM1 firmware, hardware abstractions, LCD, USB/TRS MIDI, interrupt infrastructure, OTA and recovery functionality that RUPTURE retains and adapts. Felucca-derived material remains subject to **GPL-3.0-only** terms and attribution.
- **M-VAVE:** original FM1 hardware and stock firmware. **JieLi:** AC791-series SDK and toolchain. RUPTURE is unofficial third-party firmware, not an M-VAVE release.
- **Inherited upstream attributions and third-party notices:** Electrosmith/DaisySP; Emilie Gillet (including Rings); Google, Pascal Gauthier and msfa (Dexed); Hügelton/Fukiai; Inter Project Authors; DotGothic16 Project Authors; Versilian Studios/VCSL; Tony Gies/klattsch; charlesvestal/X0X. These credits reflect material and notices inherited with upstream sources, not a claim that every component contributes to the RUPTURE CXS audio path.

See `LICENSE`, `licenses/FELUCCA-LICENSING.txt` and the original licenses/notices under `vendor/etch-v1.88-source` for the applicable full terms, contributors and copyright notices. All relevant upstream credit files are retained in the complete source archive.
