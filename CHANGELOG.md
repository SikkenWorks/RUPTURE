## V1.40 — FX CPU budget and default EQ bypass (2026-11-01)

- Fixed the shared post-mix FX scheduler: disabled chorus, delay and reverb no longer process full wet DSP at MIX 0 when DRIVE, OCT FUZZ or another effect is on. Their current smoothed MIX remains authoritative, preserving audible fade-outs and original algorithms.
- Optimized the RUPTURE eight-effect dispatch to bypass dormant legacy ETCH modulation engines while retaining the full path when a legacy project uses them.
- Removed a compressor toggle click: disengaging compression now releases its gain smoothly instead of jumping instantly to unity.
- New projects start with Channel EQ 2.0 selected but fully bypassed (OFF), with neutral gain parameters. Existing saved project EQ state and bypass mask remain unchanged when loaded.
- No changed bootloader, OTA loader, storage structure, filter algorithm or DMA safety cutoff. Physical FM1 timing and stability are not yet validated for this revision.

# RUPTURE V1.39 — Dry Audio Stability Fix

- Fixed Channel EQ 2.0 at 0 dB incorrectly preventing the global FX dry fast-path. It previously forced non-trivial EQ and effect processing even with all filters/FX off.
- Flat EQ2 coefficients and inactive bands now bypass their heavy realtime calculations while preserving active-EQ processing, stored settings and parameter transition behavior.
- Fast neutral pitch-bend and filter-OFF paths reduce unnecessary operations for RUPTURE's monophonic CXS engine.
- Native FM1 build and targeted host regression tests pass. A physical FM1 DMA soak test remains required; this is not a claim of independently measured elimination of crackling.
- The DMA buffer ownership protection, audio-stop guard, protected boot header, trusted OTA loader and project storage format are unchanged.

# RUPTURE V1.38 — Full filter optimization & monophonic DMA protection

- Prepared and cached filter coefficients for LOWPASS, BANDPASS, HIGHPASS and 24 dB/oct LADDER. Stable controls no longer recompute the same coefficients for every sample. The filter output and internal state remain sample-exact against previous kernels.
- Corrected the DMA overload response for RUPTURE's actual one-voice architecture: ineffective 8→2 voice shedding is replaced with immediate nonessential UI/scope throttling. Persistent genuine missed deadlines still trigger the audio safety stop.
- Kept DSP pole counts, cutoff/resonance mapping, fixed-point gain, project files (PJT34), effects, EQ2, panel controls, USB, flash layout and trusted OTA loader unchanged.
- Native crossbuild and 2.5 million new sample/state comparisons pass. This does not replace physical FM1 deadline, audio and power-on stress tests.

# RUPTURE V1.37 — Ladder realtime stability and audio fault diagnostics

- Replaced five signed 64-bit multiplies per sample/voice in the resonant four-pole ladder hot path with overflow-bounded 32-bit Q15 arithmetic. The resulting output and all four internal states are exact matches to V1.36 for valid audio input, including maximum resonance.
- Added Q15 input bounding in the ladder branch to prevent upstream values outside the documented audio range from destabilizing the feedback loop. Ordinary signals are unaffected.
- Kept audio DMA deadline preflight, voice-budget relief, audio-stop protection, boot/recovery behavior, project formats and stored sounds intact. The audio watchdog is **not disabled**.
- A deadline-induced audio shutdown now shows **AUDIO STOP / DMA TIMING FAULT** rather than incorrectly presenting it as a boot SAFE MODE. Genuine Safe Mode remains available with its original meaning.
- Completed native JieLi crossbuild and 972,000 sample-exact filter comparisons, including 300,000 automated parameter-sweep samples. Physical FM1 measurements remain necessary before declaring the reported crackle and shutdown fixed on hardware.
- Preserved the original protected 16 KiB flash header, OTA-loader bytes, firmware identity `FM-1_968` and existing PJT34 storage format. All features from V1.36 retained.

# RUPTURE V1.36 — Storage recovery and Auto Load Project

- WIPE ALL uses a dedicated confirmed and address-restricted native NOR erase/readback path over known user flash 0x97000..0xDFFFF. Standard project writes remain confined to the existing RUPTURE A/B banks. Boot, application and OTA sectors remain protected.
- Fixed deletion of the last saved project in the dedicated project bank pair; an unsaved NEW project can be created afterwards. Shared-waveform-owner deletion while surviving projects depend on it remains safely blocked and now explains why.
- Rejected project edits no longer permanently poison the storage state. Writable state is restored only when a committed valid project catalog is readable.
- Added GLO → SETTINGS → AUTO LOAD PROJECT. It defaults to NEW; SELECT browses only existing projects plus NEW; SEL chooses the power-on target. Missing/deleted targets fall back to NEW. The choice is CRC-protected in the existing 8-byte device-global flash journal and survives regular save and bank compaction.
- Preserves Channel EQ 2.0, PJT34 compatibility, project and sequence data format, audio DSP, protected 16-KiB bootheader and original trusted OTA loader.
- Native FM1 crossbuild and directed host NOR storage tests passed. Flash behavior and cold-boot AUTO LOAD still need physical FM1 verification.

- **V1.35:** EDIT cycles EQ GAIN / FREQUENCY / Q, SEL no longer cycles.
# RUPTURE V1.34 — Channel EQ 2.0 and parametric peak width

- New fixed-point post-mix four-biquad EQ: LOW 80 Hz shelf, BODY 250 Hz bell, BITE 1.5 kHz bell and AIR 5 kHz shelf, all with ±9 dB gain in 0.5 dB steps.
- EDIT opens GAIN / FREQUENCY / PEAK WIDTH (Q); repeated EDIT presses cycle the pages, never SEL. K1–K4 adjust the four gains or center frequencies; K2/K3 adjust BODY/BITE Q from 0.3 to 8.0. Lower Q is wider; higher Q is narrower. EQ2 response plot uses the actual coefficients.
- Gain is bit-identical at 0 dB; coefficient smoothing and post-boost headroom management minimize clicks and clipping. Biquads are implemented in fixed point without per-sample transcendental calculations.
- New projects use EQ2. Older projects load LEGACY by default, with optional ALGORITHM switch to EQ2. PJT34 stores EQ mode, four frequency selectors and both Q values without enlarging the known flash erase layout. Previous PJT33 records remain decodable.
- Original OTA loader, protected flash header, other effects, sequencer and MIDI remain unchanged. Native build and host regressions pass; physical FM1 CPU, sound and flash tests still required.

# RUPTURE V1.33 — Sequence save/load and UI alignment

- Fixed the first-save sequence bug: newly named empty slots no longer trigger a false overwrite confirmation before note data is committed to flash.
- Saving a sequence now persists its BPM and step length per sequence in existing patch metadata, with no project-format expansion. Loading restores the saved pattern, BPM, length, rate, gate and swing.
- Older sequences without the new metadata marker use stored project timing as a fallback. A save that failed in a previous release cannot be retroactively recovered if its pattern was never written.
- Legacy sequence name placeholders `ETCH24`–`ETCH31`, with or without a space, start with an empty name field; existing populated patterns and custom names are retained.
- The PROJECTS and SEQUENCES save/open browsers show the opened entry with `ACTIVE`, separately from the selection highlight.
- Both small pitch-octave and HOLD readouts use the same centered value column on the native 240 × 240 sequencer screen.
- Preserved the CXS audio DSP, storage project journal, first 16 KB bootheader and trusted OTA loader.

## V1.22 · FX stability, mixer & fifth SUB knob

- Removed WAVEFOLDER and TRANSIENT SHAPER from RUPTURE FX browsing and audio rendering. Remaining FX: eight. Old saved settings remain decode-compatible; legacy code is still present for ETCH compatibility.
- Octave Fuzz: dry/wet envelope-based loudness normalization after distortion, preserves full Drive and Octave intensity.
- Tilt EQ: stronger, independent K1 Tilt, K2 Frequency, K3 Mix, K4 Output.
- Sandpaper: independent and audible SOFT/WARMTH ranges, smoothed controls and reduced parameter-switch transients.
- Channel EQ K4: HIGH CUT replaced with HIGH AIR treble boost (0 neutral); historical K4 values adopt new meaning.
- SUB 1 and SUB 2: fifth virtual BLEND knob, physical ALGORITHM control, independent gain 0–100; saved per project inside existing 21-byte PJT33 extension with new tag; older PJT33 blobs load with default BLEND 75.
- Important downgrade warning: newly saved V1.22 PJT33 records cannot load on V1.21 or older. Keep backups; real FM1 stability unverified.

## V1.21 — SAVE project menu on every HOME page
- Fixed a native navigation omission: SAVE now opens the same named 12-slot PROJECTS browser from MAIN, SOUND SHAPE, SUB OSC 1 and SUB OSC 2. Previously SUB 1 and SUB 2 ignored the button.
- SEQ / SEQ SLOTS still route SAVE to the independent eight-sequence library. The FX+SAVE service chord remains reserved. Project writes still require explicit browser actions and confirmation; no automatic flash write is added to SAVE navigation.
- Added native shared SAVE routing and regression coverage for all four HOME pages, non-home behavior and the protected chord. No changes to CXS DSP, FX, audio DMA, flash schemas or trusted OTA loader.
- Updated the official `index.html`, README, CHANGELOG, cheat sheet and embedded installer together with firmware V1.21. The physical button test on FM1 remains outstanding.

## V1.20 — Unified information and installation website
- Replaced the separate installer entrypoint with a professional `index.html` containing auto-loaded README.md / CHANGELOG.md, a searchable cheat sheet and the original safe Web MIDI installer.
- Added embedded offline documentation snapshots and one-click download of the SHA-256-validated firmware. The page always uses the release file when served via GitHub Pages, so routine documentation edits update without rebuilding the firmware.
- Added deterministic website generation with a strict `.fwsc` / embedded installer / release-manifest SHA-256 parity check. The OTA loader and firmware image are unchanged by this website update.

## V1.20 — RUPTURE / SIKKENWORKS Animated Boot Intro
- Replaced 10-stage pixel-assembly boot logo with 24-frame pure 1-bit jagged crack / widening rupture intro; final exact wordmark and slogan: SIKKENWORKS / Born out of passion.
- Single consistent 240×240 art source prevents mismatched crack geometry from frame to frame; all intermediate frames are generated by the actual native C UI engine from a 7,200-byte read-only bitmap.
- Runs exclusively before audio init; no additional per-sample CPU work, video buffer, heap, data-flash changes or modified trusted loader. FM1 watchdog fed during the reveal and final hold.
- Existing 1-bit screens, 12×8 named storage, ten FX, CXS/sub mix and hardware-recovery priority retained. Host tests verify intermediate 1-bit PNGs and final bitmap parity. Physical FM1 boot timing not yet tested.

## V1.19 — Realtime DMA Telemetry and Priority for Audio
- Added GLO → AUDIO DIAG: WALL MAX, DSP MAX, RENDER, PUBLISH, MIN SLACK, LATE, GUARD and SCOPE SKIP. All are read-only, update at most once per second on screen, and are reset at audio init.
- The ALNK interrupt now times core synth+FX rendering separately from DMA/USB/visual scope publishing. Each counter is a fixed-size scalar; no audio heap, memory copy, printf, file I/O or flash write added to realtime work.
- Under high ISR wall-time pressure the visualization capture is deferred rather than contending with audio DMA and USB. This never drops an audio or MIDI frame. A higher SCOPE SKIP count is a visual-performance indicator, not an audio failure.
- Regression test for DMA preflight, interrupts, timer wrap, visual pressure thresholds, as well as all previous twelve-project and ten-FX stress tests.
- Protected first 16 KiB, OTA loader, PJT33 structure and the entire audible CXS and FX DSP algorithms remain unchanged. Real-hardware DMA deadline tests are still necessary.

## V1.18 — FX Full-Chain Peak Guard & DSP Hot-Path
- FX-bus soft knee now engages for the compressor, wavefolder, transient, tilt, octave fuzz, Sandpaper and EQ as well as time-based effects. Maximum wet PCM peak is 28,000/32,767, with a division-free knee starting at 22,000. Full neutral bypass remains bit-exact.
- Removed compiler-emulated signed 64-bit divides from the hot per-sample processing of the four newer FX, Sandpaper blending, and echo/reverb feedback. All narrowed products have explicit signed 32-bit range bounds. FX feedback remains bounded and unchanged in the A/B storage format.
- A hard DMA preflight remains enabled: software-only host tests cannot guarantee that an FM1 at maximum FX will never overrun. On-device worst-case TIMER4 wall-time and extended power/USB load soak remain REQUIRED. This release does not disable the protective audio-stop behavior or suppress diagnostic counters.
- Strong Wavefolder and Octave Fuzz drive each uses a small pre-nonlinearity one-pole filter (8 B total additional DSP state) to reduce high-frequency alias energy without 2x CPU oversampling. This is mitigation rather than alias-free synthesis.
- All existing 12-project PJT33 storage, FX settings, project SAVE/autosave, and boot/OTA sectors remain compatible. The limiter protects final PCM, not nonlinear aliasing already generated by wavefold/fuzz at Nyquist; a full hardware quality proof remains pending.

## V1.17 — FX Memory & Project Compatibility

- Introduced PJT33: a bounded **21-byte** project-snapshot extension for Transient Shaper, Tilt EQ, Octave Fuzz and Sandpaper, all sixteen K1–K4 settings, their four enabled flags, and the existing 22-bit FX bypass mask.
- Manual project SAVE and guarded 20-second autosave capture this FX state before the existing CRC-checked A/B journal commits; no flash operation is added to the audio interrupt path or another sector.
- LOAD and cold boot restore the correct project FX. Old PJT32 projects restore all four new FX to OFF with documented neutral parameter defaults and bypass reset, preventing cross-project leakage.
- Bad, missing or truncated PJT33 extension data is rejected. Host regression tests include roundtrip, corrupt masks/knobs, legacy format migration, project isolation, and all prior firmware tests.
- Existing 33-byte per-preset DSP record remains unchanged; the 12 named project / eight sequence catalog and protected first 16 KiB + trusted OTA loader remain unchanged.
- PJT33 projects **are not backward-readable in V1.16 or older firmware**. Back up projects before a downgrade. Hardware FM1 storage/OTA validation and audio CPU profiling remain pending.

## V1.16 — Sandpaper
- Added SANDPAPER as the tenth native RUPTURE FX selection with its own 1-bit screen and K1 SMOOTH, K2 SOFT, K3 WARMTH, K4 MIX.
- New stereo-linked post-CXS signal smoothing uses a 2-stage IIR low-pass, gentler peak knee, controlled bass-body shelf and smoothed dry/wet fade. Three tiny filter states, no delay buffer, no heap and no flash changes.
- Host DSP tests verify 95 Hz bass retention, 4.5 kHz harmonic reduction, knob response, bounded full-scale audio, exact dry bypass after fade. The established V1.15 FX tests remain passing.
- Sandpaper defaults OFF, settings session-only, physical FM1 listening and CPU load profiling still pending.
- Native pi32v2 firmware, 37 bit-exact 1-bit renderer exports and firmware WebInstaller.

## V1.15
- Added COMPRESSOR and WAVEFOLDER to RUPTURE FX (SEL toggle, SELECT scroll, four native knob parameters). They use the existing persisted native FX settings and no additional sample buffers.
- Added audible post-CXS TRANSIENT SHAPER (ATTACK/SUSTAIN/SENS/MIX), TILT EQ (TILT/FREQ/MIX/OUTPUT) and OCTAVE FUZZ (DRIVE/OCTAVE/TONE/MIX) in native fixed-point DSP.
- Each of the five FX is independently enabled/bypassed. All default off; old dry audio is bit-exact at defaults. New algorithms have bounded state, no heap and a short mix ramp on bypass.
- 36 true 1-bit native screen exports, project and existing OTA flash map unchanged. Native audio host tests added for all five effects.
- Transient/Tilt/Octave settings remain session-only pending storage format and backwards-compatibility validation; physical CPU/audio stress remains unverified.

## V1.14
- Introduced per-effect click-reduction ramps for chorus, delay and reverb on bypass toggles.
- Maintained exact dry CXS output, unchanged flash layout, recovery loader and user project format.
- Added a linker-section SRAM and FX recommendation audit. Hardware timing remains unverified.

## V1.13
- Device-global Master Volume persists independently of project/preset loads using a CRC-validated append-only NOR record.
- Verified project bank swap preserves volume; exhausted journal safely compacts with verified A/B commit.
- Fixed native 296-byte PJ52 catalog write to comply with the FM1 HAL 256-byte maximum and page boundaries.
- Corrected boot-time resetting of Master Volume to 100%.
- CXS bass, dual subs, existing project/sequence storage and trusted OTA-loader preserved.
- Host regressions and native JieLi build pass; physical device test still pending.

## V1.12
- Store and restore NOTE MEMORY (LAST/HIGH/LOW, LEGATO, GLIDE 0–400 ms in 5 ms increments, BEND 0–12) with each saved CXS preset/project snapshot.
- Master Volume 0–200% now survives project SAVE/LOAD and restart for the saved active CXS preset. The value is **project/preset-scoped**, not a device-wide separate global setting.
- Old snapshots without V1.12 marker restore safe defaults (100%, LAST, LEGATO ON, GLIDE 0, BEND 2).
- No new flash sector, project codec version, or change to 33-byte PJT32 records.
- Original OTA loader and 16 KiB protected header preserved; hardware validation still required.

## V1.11 — Dual Sub Preset Memory
- SUB 1 and SUB 2 now persist TYPE, OCTAVE, DETUNE, DRIFT per saved preset and project.
- Legacy presets without a sub marker load both subs OFF with independent neutral octave/detune defaults.
- Extra tag validation rejects orphaned/malformed sub records; existing PJT32 record size and A/B flash bank layout unchanged.
- SOUND SHAPE memory and original CXS-off output retained.
- Native pi32v2 build and host roundtrip tests added. Physical flash/reboot verification still open.

## V1.10 — Suboscillator label + audibility fix

- Corrected both suboscillator knob label rows to TYPE / OCTAVE / DETUNE / DRIFT (missing NAME EDITOR label row had shifted array indices).
- Replaced attenuating sub crossfade with additive sub summing that maintains 100% main CXS level while subs are enabled.
- Increased sub contribution limit to 50% each, with bounded peak guard above 28k Q15.
- SUB 2 defaults to -1 octave instead of -2 to improve audibility on typical audio playback; -2 remains selectable.
- Independent SUB 2 default detune +4 cents; CLASSIC upgraded from pure sine to a harmonically band-limited square-like oscillator; TYPE remains OFF by default to preserve earlier patches.
- Added focused regression tests for both UI labels, no-level-drop mix, per-sub harmonic energy, bounded peaks and both-OFF parity.
- No persistent storage/OTA changes; physical listening and CPU measurements remain outstanding.


## V1.9 — Dual independent suboscillators

- Added SUB OSC 1 and SUB OSC 2 as dedicated HOME knob pages after SOUND SHAPE.
- Both subs offer independent TYPE OFF/CXS/CLASSIC (sine), OCTAVE −2/−1/0/+1, DETUNE ±50 cents, and DRIFT 0–100.
- Native CXS sub generators use separate exchange states and random seeds; CLASSIC uses band-limited sine lookup. Both share the played monophonic note, glide, and MIDI pitch.
- Bounded pre-filter gain compensation prevents summing three full-scale generators; short fades protect TYPE changes. Both OFF preserves the former main CXS signal.
- Added host synthesis tests and 31 native C-renderer pixel-perfect screens. Native crossbuild and .fwsc validation pass.
- Sub parameters currently session-only; no project format changes. On-device tests remain open.


## V1.8 — Named libraries, SAVE context and keyboard step playhead

- SAVE now opens PROJECT LIBRARY from PLAY/SOUND SHAPE, or SEQUENCE LIBRARY from SEQ, without performing a silent immediate write.
- New 12-project and 8-sequence scrolling text lists, with LOAD / SAVE / NAME actions.
- Implemented a 12-character on-device name editor using SELECT, ALGORITHM, SEL and HOME.
- Project titles remain in their existing 12-slot catalog; sequence titles use reserved 24–31 patch name fields already serialized in PJT32.
- Native white-key sequencer position LEDs: sixteen steps per page, dim availability and full-brightness active step. The SEQ screen follows the actual playing step.
- Codec roundtrip name persistence and white-key mapping tests passed; FM1 native build, .fwsc verification and OTA parity checked.
- Physical hardware validation still required; older RUPTURE versions may ignore reserved sequence titles.


## V1.7 — SAVE, STEP VELOCITY AND IDLE AUTO SAVE

- Restored the physical SAVE key (button bit 9) in RUPTURE's native UI routing. Physical REC (bit 13) is now independent and edits only SEQ recording.
- SAVE directly commits the current active project through the bounded, CRC-verified two-bank project writer; on-screen footer confirms success or failure, with no silent inaction.
- K3 edits the velocity of every existing note on the currently selected sequence step (1–127), and controls the starting velocity for an empty step.
- SEQ opens with an immediately editable step, without requiring initial scrolling; keyboard note-on events can edit the selected step even during playback.
- Added delayed idle-only Auto Save after the initial explicit project SAVE or LOAD: >=20 seconds since the last edit, >=60 seconds between writes, only when sequence playback is stopped, no held keys, and the storage is owned, valid and ready. Failed writes are not auto-retried without a new change.
- USB lightning icon is now keyed to configured USB connectivity rather than momentary suspended state. USB power without enumeration remains unverified as a charge indication.
- Increased active native screen refresh responsiveness, preserving the native 1-bit renderer and the original OTA loader/bootheader.
- Built native FM1 image, verified .fwsc checksum, host regression tests; no physical FM1 validation performed.


## V1.7 — SAVE, STEP VELOCITY AND IDLE AUTO SAVE

- Restored the physical SAVE key (button bit 9) in RUPTURE's native UI routing. Physical REC (bit 13) is now independent and edits only SEQ recording.
- SAVE directly commits the current active project through the bounded, CRC-verified two-bank project writer; on-screen footer confirms success or failure, with no silent inaction.
- K3 edits the velocity of every existing note on the currently selected sequence step (1–127), and controls the starting velocity for an empty step.
- SEQ opens with an immediately editable step, without requiring initial scrolling; keyboard note-on events can edit the selected step even during playback.
- Added delayed idle-only Auto Save after the initial explicit project SAVE or LOAD: >=20 seconds since the last edit, >=60 seconds between writes, only when sequence playback is stopped, no held keys, and the storage is owned, valid and ready. Failed writes are not auto-retried without a new change.
- USB lightning icon is now keyed to configured USB connectivity rather than momentary suspended state. USB power without enumeration remains unverified as a charge indication.
- Increased active native screen refresh responsiveness, preserving the native 1-bit renderer and the original OTA loader/bootheader.
- Built native FM1 image, verified .fwsc checksum, host regression tests; no physical FM1 validation performed.


## V1.5 — TWELVE PROJECT CATALOG

- Upgraded the RUPTURE owned-flash P05 catalog from 4 to 12 slot addresses within the existing two 80 KiB banks; 8 sequence archives remain per project.
- Added new catalog version 13 (PJ52), while reading and migrating existing version 11/12 four-slot catalogs without changing the already committed bank until the new CRC-checked payload is committed.
- Added true 16-bit occupancy mask and 12-entry project names; native PROJECTS navigation and the 1-bit menu now expose slots 1–12.
- A failed write or exhausted bank capacity returns an error before commit; the last valid snapshot remains available. Twelve large fully populated projects cannot all be guaranteed to fit in 80 KiB per bank.
- Added native flash-emulator tests for all 12 records, legacy four-slot migration, corrupt/torn write fallback and slot deletion; run together with prior regression tests.
- Retained CXS, soundshaping, note memory, effect chains and bass register unchanged. Preserved the original OTA loader and protected 16 KiB flash header.
- WARNING FOR DOWNGRADE: V1.5 saves catalog version 13. Earlier RUPTURE versions do not read that new catalog. Make a project backup before installation and do not downgrade without exporting/restore preparation.
- Native build and container valid offline. Physical FM1 installation and power-cycle tests remain outstanding.

## V1.4 — SOUND SHAPE PRESET MEMORY

- WEIGHT, EDGE, BITE and PUNCH now persist per preset and per saved project in the existing PJT32 per-preset DSP bank.
- Reuses four bytes formerly assigned to the ETCH Organic oscillator, which RUPTURE CXS never executes. Tagging uses the reserved high bit of spectral_wave only for RUPTURE-owned patch records; no new flash areas, sector erase policies, snapshot sizes or boot components.
- Upgraded the RUPTURE project-capture path to PJT32, preventing accidental downgrade of saved shape data to PJT30.
- Legacy/older RUPTURE projects with no shape tag restore neutral 0/50/0/0 settings, avoiding invented values.
- The legacy organic pitch-drift data is never interpreted as CXS shape controls and is bypassed for RUPTURE playback.
- Extended project codec validation, rejecting shape markers on non-RUPTURE presets; added encode/decode, backwards-compatibility and corruption-guard host tests.
- Native pi32v2 app and firmware container rebuilt; byte-identical native rebuild tested. Same FM-1_968 identity, protected first 16 KiB and original OTA loader.
- Four hardware-backed projects and 8 sequence slots each remain; 12 project slots, TRS MIDI OUT and native loader-screen progress remain unimplemented.


## V1.3 — CXS LFO + SOUND SHAPE

- Routed the actual LFO directly into monophonic CXS audio: KEYED/FREE/TEMPO with PITCH vibrato and LEVEL tremolo targets. Previously the UI changed an unused wavetable modulation path.
- Short HOME on PLAY opens a second native four-knob page: WEIGHT, EDGE, BITE, PUNCH. HOME again returns to the main CXS controls; long HOME returns to PLAY.
- Added four bounded fixed-point CXS sound-shaping processors, all directly controlled by hardware K1–K4. Neutrals preserve the previous dry CXS waveform; EDGE 50 preserves original smoothing.
- Replaced the header font with a dedicated heavy 8x9 pixel RUPTURE wordmark drawn by the native 1-bit C renderer.
- 28 screen exports from exactly the same native firmware renderer, including SOUND SHAPE.
- Added PCM-difference, conserved-cycle and output-range host checks for the new LFO and shaping controls.
- Updated firmware identity metadata, WebInstaller payload/hash and flashable `.fwsc`. Protected bootheader and OTA loader remain byte-identical to the validated reference package.
- Flash storage layout remains unchanged: 4 native project slots with 8 sequence slots per project. Extra SOUND SHAPE controls are runtime-only pending safe storage migration.

## V1.2

- CXS PRESSURE/BALANCE/MEMORY/DENSITY now survive preset and project save/load without changing project-record size; legacy patches remain readable.
- Added native 31250-baud TRS MIDI IN over UART1, RX-DMA ring and channel voice/realtime parser.
- Interleaved USB and TRS messages use the existing bounded audio MIDI queue; overrun triggers panic instead of stuck notes.
- Added running-status, MIDI clock, sustain CC, pitchbend, system-common, and SysEx-discard tests.
- Fixed onboard HELP: 4 available project slots, not 12 already present.
- Kept the protected boot header and original trusted OTA loader intact.
- Not implemented: native twelve-project flash expansion, TRS MIDI OUT, separate loader LCD progress, proof of charging current, physical FM1 validation.


## V1.1

- Corrected the virtual potentiometer geometry: 0 at 7:30, 50 at 12:00, 100 at 4:30; smooth interpolation retained.
- Fixed RUPTURE LFO page to control existing C-wave modulation RATE, DEPTH, MODE and TARGET.
- Drive CUTOFF automatically enables the ladder filter when the selected filter was OFF, so the control has an audible effect.
- Channel EQ K4 clearly labeled LOW CUT; its DSP high-pass filter acts only on bass frequencies at the left of the curve. LOW/MID/HIGH gains remain independent.
- Battery icon adds a lightning symbol when the USB data interface is configured. This indicates USB connection, not verified charging current.
- Installer progress reports actual firmware bytes served during the independent trusted OTA-loader stage and remains visible across handoff; no fake progress is shown for unobserved steps.
- Unchanged trusted bootloader, protected flash header, CXS bass sound, project layout and MIDI protocol.
- Updated documentation, 1-bit screen captures and release hashes.


## V1.0

- Corrected ARP mode display so internal mode 0 renders OFF, not UP; the remaining mode names now match their native DSP behavior.
- Implemented the native step-sequencer UI: SELECT scrolls steps; the keyboard adds/removes per-step note triggers when stopped. REC enables live capture to the active playhead.
- K1 selects 4/8/16/32/64/128 steps, K2 adjusts the shared 30–240 BPM, K3 sets new-note velocity and K4 sets step gate.
- Updated native patch restore to recognize stored four-step sequences instead of silently reverting to sixteen.
- Channel EQ opens with EDIT, not REC. REC returns to its recording function on the SEQ screen.
- Boot logo renders immediately after LCD init, before subsequent lengthy startup and storage initialization; 1.5-second finished-logo hold retained.
- Removed the redundant fixed "K1 K2 K3 K4" screen caption, including in onboard HELP, without removing labeled parameter controls.
- Removed the spurious `README.me`. README contains product and full credits; version history is only in CHANGELOG.
- Rebuilt the FM1 application and exact-pixel 1-bit screen exports; retained protected bootheader and verified OTA loader.


## V0.9

- Corrected virtual K1–K4 dial rotation (positive knob movement now moves clockwise) using the same native C pixel renderer for the FM1 and PNG exports.
- FX: SELECT cycles REVERB, DELAY, CHORUS, MOSFET, SEL toggles the focused effect; K1–K4 edit its live DSP parameters. Enabling a zero-mix effect supplies a safe audible starting mix.
- Reverb SPRING / HALL / ROOM offer distinct functioning decay/size presets within the inherited DSP core; physically modeled spring tank not claimed.
- REC opens native black/white Channel EQ with a live response graph, 3 gain bands and HPF, all four parameters wired to native DSP.
- SETTINGS now shows only working K1 MASTER VOLUME, no unused K2–K4 controls.
- Battery icon uses Felucca ADC voltage thresholds 531/561/591 with smoothing and valid-reading checks; no fabricated percentage. BEETLE battery UI approach credited as adaptation.
- Updated HELP, README, CHEAT SHEET, installer, native PNG exports, tests and flash package. Preserved native CXS, OCT 0 baseline, six-note memory, project data sectors, boot header and trusted OTA loader.


## V0.8

- Added native **8 sequence slots per verified project** to RUPTURE, backed by the existing CRC-checked P05 A/B project snapshot. No new flash regions.
- Long-press SEQ opens a fully functional LOAD/SAVE screen; SELECT picks a sequence slot and ALGORITHM chooses action; SEL enters, double SEL confirms SAVE. K1 SLOT, K2 ACTION, K3 RATE, K4 SWING.
- Saved sequences retain notes, holds, velocity, lengths, parameter locks, rate, gate and swing. Loading stops transport before switching pattern and avoids changing live data on corrupt archive entries.
- Wired native PROJECTS screen to actual four existing project slots with explicit load/save confirmation. Unverified slots 5–12 are not exposed as writable.
- Added explicit save status and capacity/failure feedback; corrected status text to avoid implied success.
- Preserved CXS audio, OCT 0 pitch, 6-note memory, boot header and trusted OTA loader.
- Added host regression for all eight saved sequence patterns and corruption isolation; regenerated 26 exact native 1-bit screen PNGs.

## V0.7

- Made the former absolute OCT −2 register the RUPTURE baseline **OCT 0**.
- RUPTURE OCT 0 uses exactly the same oscillator pitch and key mapping as V0.6 OCT −2.
- OCT− and OCT+ LEDs now indicate the octave **relative to RUPTURE 0**; neither stays lit at baseline.
- Main-screen and other RUPTURE headers explicitly show `OCT 0`; shifting OCT+ once shows `OCT +1`, OCT− once shows `OCT -1`.
- Physical octave travel remains unchanged (absolute −4..+3, shown as RUPTURE −2..+5).
- Tested the display translation, LED status, press limits and unchanged bass-note MIDI mapping; regenerated exact native UI PNGs.
- No changes to CXS synthesis, note memory, filters, bootloader, OTA loader or existing patch storage.

## V0.6

- Introduced the working low bass register (then called OCT −2), corrected OCT buttons and LEDs, and improved encoder polling, knob animation and boot animation.

# RUPTURE V1.32 — Utrecht screensaver, settings & REC LED

- Removed the square R monogram from the GitHub Pages website header; SIKKENWORKS text remains.
- Replaced the meadow screensaver with original author-supplied monochrome Utrecht artwork (Domtoren and city buildings) and a staged street fracture that expands into a canyon. Uses only the existing 240×240 UI framebuffer; audio DSP and USB updater unchanged.
- SETTINGS → OTHER → SCREENSAVER now opens a dedicated options page: ENABLED toggle with SEL, START AFTER adjustable by K1 when selected with SELECT. Values: 15, 30 (default), 45, 60, 90, 120, 180, 300 seconds.
- A/B global preference record remains 8 bytes, with backward-compatible decoding of all prior autosave/saver settings and new persistent delay index. Device-global settings survive project switches and volume changes.
- REC function-button LED is OFF by default and illuminates only when live sequence recording is armed. It extinguishes when recording is toggled off or transport stops.
- No audio engine, storage region/layout, protected bootheader or trusted OTA loader changes.

# RUPTURE V1.31 — Original QR restoration

- REPORT A BUG now embeds the **exact 29x29 module bitmap from the author-uploaded QR image**, preserving all 841 modules. Reconstructed 29-module QR in V1.30 deviated on six modules and used reversed colors.
- The original supplied white-on-black QR is rendered without interpolation at 5x5 FM1 pixels per module, centered in its own screen.
- The original uploaded PNG is preserved unchanged in `assets/qr-code-original.png` with SHA-256 verification and source extraction script. No QR regeneration.
- All previous RUPTURE functions, persistent storage formats, DSP, bootheader, and OTA loader remain unchanged.

# V1.30 — Live STORAGE catalog, legacy sequences, pixel meadow and bug-report QR

- STORAGE → PROJECTS now explicitly refreshes the native catalog on entry. The list no longer depends on first visiting the normal PROJECTS page.
- Empty legacy ETCH factory-labelled 24–31 patch slots are ignored only when no pattern data exists. Saved sequences with real patterns or deliberate custom names remain visible; no stored data is changed by this display fix.
- MASTER VOLUME values and AUTO SAVE ON/OFF are vertically centred in their SETTINGS cards and use a consistent right-hand value area.
- 30-second idle screensaver becomes a deterministic meadow and widening pixel ravine, with cliff edges, strata, grass, branching fractures and falling fragments. Rendered using the existing 240×240 1-bit UI canvas, not the DSP. The final frame holds instead of snapping back.
- GLO now adds REPORT A BUG as a fifth card, opening a sharp embedded 29×29 QR symbol which scans to https://github.com/SikkenWorks/RUPTURE/issues and reads SCAN TO REPORT A BUG below it.
- Bootloader/OTA loader, protected first 16 KiB, DSP and flash storage schema preserved. Native build and tests have been run; physical checks on FM1 remain necessary for changed behavior.

# V1.29 — Global menu labels and direct SETTINGS controls

- GLO now displays **AUDIO DIAGNOSTICS** in full, without truncating the card label.
- SETTINGS > AUTO SAVE is a direct checkbox: SELECT chooses its row and SEL toggles ON/OFF immediately. No separate screen. The choice retains the existing persistent global journal behavior and defaults OFF.
- SETTINGS > MASTER VOLUME allows K1 to adjust the percentage directly when its row is highlighted; SEL still opens the dedicated volume page and there SELECT or K1 adjusts the same 0–200% control.
- SETTINGS > OTHER > SCREENSAVER remains a direct SEL toggle with no separate page, ON by default.
- UI-only navigation improvements; existing audio engine, data layout, OTA loader and first 16 KiB bootheader are unchanged.
- Host tests validate rendered labels and the input routing source contracts. Physical FM1 confirmation is still required for this build.

# V1.28 — Storage Manager, ABOUT, USB indicator and refined screensaver

- Fixed only the missing center stroke of the E in the top-left RUPTURE header, leaving the other cracked letterforms unchanged.
- USB-C charging/connection bolt now follows live USB configuration/suspend/disconnect state and disappears when the cable is unplugged.
- SETTINGS > STORAGE > PROJECTS: inspect 12 saved projects; OPEN, SEQUENCES, DUPLICATE to first empty slot, RENAME, DELETE. Eight per-project sequences can be opened, duplicated, renamed and deleted through the existing native pattern archive. Operations are CRC/commit-checked without replacing a stored project with unrelated live RAM.
- SETTINGS > STORAGE > WIPE ALL: defaults NO, requires YES via SELECT+SEL, erases and verifies known user-storage/legacy sectors 0x97000..0xDFFFF only. Preserves boot, application, OTA staging and trusted loader. Irreversible. Active project and shared wave-owner protections stay in effect for individual deletes.
- SETTINGS > OTHER > SCREENSAVER: enabled by default. Choice persists in existing 8-byte A/B global preference records with old records transparently interpreted as enabled. AUTO SAVE remains opt-in OFF by default.
- Screensaver now draws branching, opening asymmetrical fissures with pixel fragments. UI framebuffer only; no DSP, audio ISR or allocated audio buffers changed.
- GLO > ABOUT: complete author-supplied English introduction, correctly word-wrapped and scrollable using SELECT on the physical 240×240 panel.
- README updated exclusively as product documentation, control guide and upstream credits, with no version/change history; this CHANGELOG contains the historical entries.
- Native storage, UI and sequence manager regressions passed in host tests. Protected first 16 KiB and trusted OTA loader stay identical to prior release. Author to verify new behavior on real FM1 hardware.

# V1.27 — Screensaver, Compact Sequencer, Global Settings and UI Controls

- UI-only 30-second idle screensaver: black panel and progressive jagged cracking, using existing 1-bit framebuffer; wakes on activity. Transport, notes, OTA, recovery and unsafe storage prevent activation. No audio DSP modifications.
- SEQ length 4/8 only draws the actual active steps; unused grid boxes are hidden. Eight- and four-step layouts are centered.
- AUTO SAVE is now opt-in and defaults OFF. The device-global setting survives power cycles, project writes and A/B log compaction using the existing 8-byte NOR records (new magic for ON, legacy records read as OFF). No project-format changes.
- GLO uses outlined cards matching HELP; SETTINGS has MASTER VOLUME and AUTO SAVE, navigated by SELECT + SEL. MASTER uses SELECT/K1 after entry; AUTO SAVE uses SELECT for OFF/ON, SEL to apply.
- Removed the generic ALGORITHM-to-K1/K2/K3/K4 fallback; explicit mappings for SUB BLEND, sequence sustain, name editing and HELP scrolling stay intact.
- README rewritten as permanent product documentation, user controls, installation, licensing and full upstream attributions. Release history is kept only in CHANGELOG.md.
- Updated native UI screens and regressions. Trusted OTA loader, bootheader, firmware identity and synthesis/FX/audio ISR unchanged. Physical validation of the new release remains pending.

# V1.26 — Slot Save/Load Semantics, Battery Calibration & OTA Transfer Progress

- Project/sequence library: SEL or SAVE on an empty slot opens the name input; SAVE on an occupied slot overwrites (with confirmation when different from the current slot); SEL on an occupied slot prompts before loading. SAVE+SEL on occupied slot retains RENAME.
- Name editor removes the redundant SEL NO ACTION footer, leaving SAVE TO STORE.
- Battery 100% interior now fills all 24 pixels; the full ADC threshold changes from 591 to 578, with the old invalid-read and smoothing protections intact. Actual battery calibration still needs checking on user hardware.
- Browser installer removes blanket claims that RUPTURE is untested on physical FM1s and tracks unique transferred bytes for meaningful progress. Device OTA loader remains unchanged: actual on-device flash progress is not claimed.
- Full original synthesis/FX/MIDI/USB/storage and existing firmware identity FM-1_968 retained. No changes to flash record layout or protected boot area.

# V1.25 — SAVE Integrity, Storage Progress, USB Branding & Bipolar EQ

- Knob spacing corrected on all standard K1–K4 pages: upper knob centers y105, lower y181; title area, lower values and footer no longer overlap.
- USB runtime descriptors now report manufacturer **SIKKENWORKS** and product **RUPTURE** instead of ETCH. The pinned updater/loader handshake and `FM-1_968` package identity are unchanged. Old host OS MIDI-port caches may need USB reconnect.
- The NAME editor now uses **SAVE only** to write. **SEL does nothing** to the name; SELECT moves the cursor and ALGORITHM edits characters. On failed SAVE, the typed name remains available for retry.
- First explicit SAVE is permitted on a verified blank RUPTURE-owned project bank, while unknown/foreign, unapproved or unsafe flash regions remain protected. Project and sequence writes use the existing CRC-verified A/B journal and report failures accurately.
- Native 1-bit storage popup overlays report actual ERASE / WRITE / VERIFY / READ progress from completed NOR operations. Sequence RAM loads are naturally nearly instant. Overlay callbacks run only between flash commands, with the audio engine paused.
- CHANNEL EQ K4 is a signed treble shelf: **-100 to +100** with neutral zero. Lowpass high-band attenuation and positive AIR boosting both work. Existing positive-only K4 projects migrate using a new marker (bit 5) in the unchanged 21-byte PJT33 FX record. Older versions do not understand this marker.
- Device OLED/LCD progress *during the separate OTA loader's flash stage* is **not implemented**: the trusted self-contained loader owns the display during the transfer. The browser's real progress remains; no unverified modifications to recovery/boot code.
- Regression coverage includes the full native audio/FX suite, first-save on erased NOR, 12 projects, A/B torn writes, progress phases, USB descriptors, signed EQ, and source routing. Real FM1 SAVE/LOAD and OTA tests still required.

# V1.24 — Dual-SUB + FX Headroom, CLASSIC Sub Fix & UI Polish

- Corrected SUB mixing under effects: small fixed-point dynamic headroom allocation, continuous soft-peak safety, and sample-rate BLEND slew. The sub-disabled CXS path stays byte-for-byte dry-equivalent.
- CLASSIC now enters after the resonant CXS filter but before ADSR, velocity and the eight-effect chain, ensuring CLASSIC does not disappear when the filter closes. CXS subs retain their filter routing. This is a deliberate sound change for CLASSIC patches.
- Tested alternating CLASSIC/CXS sub channels and changing ALGORITHM blend while eight RUPTURE effects are fully active, including delay/reverb feedback, in 110250 frames; no full-scale PCM clipping and no systematic silence in the host test.
- Moved upper knob visuals lower so the 50–80px label panels are not touched; added a 1-bit cracked RUPTURE wordmark with native-rendered output.
- No changes to PJT33 flash schema, 12 projects, eight sequences, independent FX saving, USB/MIDI, OTA loader or initial bootheader.
- Hardware validation still needed: actual FM1 DMA deadline, high-load audio and perceived loudness matching; host tests are not guarantees.

# V1.23 — Named SAVE, Sequencer Sustain & Scrollable HELP

- Fixed PROJECTS and SEQUENCES save UX: SEL in NAME editor now commits the name and the appropriate stored data; the physical SAVE key also commits. When already inside PROJECTS or SEQUENCES with SAVE selected, physical SAVE performs the save action rather than reopening/resetting the list.
- Added visible confirmation/error status in both slot libraries: SAVED, SAVE FAILED, SEL AGAIN TO SAVE and LOADED. Occupied slots require an explicit second confirmation.
- On sequencer step with a note, ALGORITHM extends native note duration across empty following steps. It stops at the next attack, sequence boundary or 255-step native limit; turning left shortens the duration. The visual grid distinguishes held steps from triggered steps.
- Reworked HELP into 10 scrollable cards with chapter titles, page counters, and scroll indicators. SELECT changes cards, ALGORITHM scrolls three lines at a time.
- Increased the size of "Born out of passion." in the actual 240×240 monochrome boot ROM, regenerating all 24 true native frames.
- Preserved PJT33 project data, CXS DSP, eight live effects, dual SUB BLEND, OTA loader and initial bootheader. All new user-visible behaviour requires physical FM1 confirmation.

# CHANGELOG
