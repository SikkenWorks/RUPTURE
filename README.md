# RUPTURE

**A monophonic bass synthesizer for the M-VAVE FM1.**  
Cycle Exchange Synthesis (CXS), a selectable bandlimited SAW oscillator, two suboscillators, modulation, sound shaping and effects.

**SikkenWorks. Born out of passion.**

## Getting started

RUPTURE runs on the compatible M-VAVE FM1 hardware. Before installing custom firmware, back up your projects and make sure you have a verified recovery method. Keep the FM1 powered and connected throughout an OTA transfer. Do not interrupt the separate OTA loader while it is writing flash.

On first launch, an on-device welcome overlay walks through backup confirmation, an optional storage format, the CRT filter preference and **CONTINUE TO RUPTURE**. On the first boot of **V1.82**, the firmware checks a new CRC-protected per-version setup marker. An older `welcome_done` flag by itself no longer suppresses the full wizard. The backup question, optional protected formatting, CRT setting and optional 43-screen Quick Tour are shown once. After finishing, a V1.82 marker is saved; subsequent normal restarts skip the full wizard. Existing valid RUPTURE storage is **never reformatted** by that wizard. No format occurs without the explicit confirmation. A change to stock V15 and back to the **exact same** V1.82 build may be indistinguishable from an ordinary restart when V15 preserves all RUPTURE data: automatic detection of that round-trip cannot be guaranteed. Reopen the Quick Tour through GLO. The marker is version-specific; future releases must allocate their own marker if their welcome needs to reappear. Answering NO to the backup question never prevents you from continuing. Formatting never starts automatically: select YES, then deliberately hold SAVE for three seconds to confirm a destructive initialization. If the storage already contains a valid RUPTURE project catalog, the wizard **does not erase existing projects**. Selecting NO skips formatting. CRT defaults to OFF unless a previous global preference exists. The wizard displays a clear, persistent formatting result: **FORMAT SUCCESSFUL**, **STORAGE ALREADY READY**, or **FORMAT FAILED**. Press SEL after reading the confirmation. The completion preference is stored with the existing device-global journal when storage is initialized; when storage is unavailable/foreign and formatting is skipped, completion cannot be saved and the wizard can reappear after reboot.

### Guided Quick Tour (first run or any time)

After the existing first-run setup, RUPTURE asks **VIEW QUICK TUTORIAL?** Choose **YES** with SELECT and confirm using SEL to begin the optional **43-screen** tour, or choose **NO** to start playing immediately. The tutorial displays the actual 1-bit RUPTURE screens with a small text overlay explaining how to reach each page and what each functional K1–K4 knob does; separate views cover all eight FX. The normal audio and projects are not edited by the walkthrough.

**Tutorial controls:** SEL advances, SELECT goes back or forward, ALGORITHM hides/shows the explanation overlay to reveal the entire underlying screen, and HOME exits immediately. The tour also shows the fifth SUB BLEND control and non-knob pages such as project management and diagnostics. It never saves, reformats, changes the sound, or performs an OTA operation. After setup has already been completed, reopen it from **GLO → QUICK TOUR**. Replaying the tour does **not** repeat the format or first-run configuration and does not write the setup journal. The first-run completion flag remains handled by the established storage setup.

The rotary knobs have a smooth, time-based indicator and brief focus markers. UI rendering is separate from the audio processor; if the device reports actual DMA misses, screen frames may be reduced temporarily to protect the audio.

### Basic navigation

- **PLAY:** Main CXS/SAW oscillator. The four knobs control PRESSURE/BALANCE/MEMORY/DENSITY in CXS, and BRIGHT/EDGE/BODY/LEVEL in SAW. In SAW mode, the BRIGHT range now starts with a warmer, filtered bass core; EDGE and the four SOUND SHAPE controls still provide aggressive sounds when deliberately raised. For rounded bass, start around BRIGHT 35–60, EDGE 40–55, with a low-pass/ladder filter and moderate resonance. This evokes the role of an analog bass synth without reproducing a specific hardware instrument.
- **SOUND SHAPE:** WEIGHT/EDGE/BITE/PUNCH. ALGORITHM switches the main oscillator. CXS WEIGHT boosts low-frequency body relative to the highs; CXS PUNCH also responds during sustained notes. Presets saved with nonzero WEIGHT/PUNCH sound different from V1.65.
- **SUB OSC 1 / SUB OSC 2:** Each sub has independent type, octave, detune, drift and blend.
- **FILTER:** Resonant filter controls.
- **ENV:** ADSR envelope controls.
- **LFO:** Synthesis modulation controls.
- **FX / EDIT:** Effects and channel EQ. REVERB now contains **three genuinely independent processing algorithms**, not SIZE/DECAY presets: **SPRING** uses a dispersive all-pass lattice and alternating-polarity feedback for metallic twang, **HALL** uses a Schroeder-style diffuser feeding a damped late-reflection tank, and **ROOM** uses separately spaced early wall reflections with a short comb tail. K1 selects the engine WITHOUT resetting your current parameters; K2 controls DECAY, K3 SIZE and K4 MIX. MIX 0 is unchanged dry signal, MIX 100 is 100% processed wet return. The three engines share the existing fixed 4096-sample memory and 64-sample diffuser, but only one algorithm executes at a time, globally after voice mixing. No extra synth voice or audio buffer is allocated. Each patch and project now remembers the reverb engine choice via unused bits of the existing routing setting. Old projects still load (legacy route 0/1 maps to HALL), but older firmware releases may reject projects saved with SPRING or ROOM selected, so back up before downgrading.
- **SEQUENCER / ARP:** Pattern and arpeggio features. In the stopped sequencer, SELECT chooses a step (STEP 1 is the first step); tap the same musical key to toggle its note, or press SEL to clear the entire selected step, including notes, HOLDs and parameter locks. Linked HOLD events on later steps that refer to these removed triggers are also cleared; other steps are preserved. REC never starts transport on its own. If the sequence is already playing, pressing REC immediately begins overdubbing as before. If playback is stopped, press REC to arm, then PLAY to hear a two-bar (eight quarter-note) metronome count-in. The sequencer grid and SEQ LED pulse with each beat, then playback and recording begin together on step 1. PLAY or REC during count-in cancels it without recording any notes. The keyboard stays playable during the countdown. During active REC the keyboard overdubs rather than erases, and holding a key over several steps records sustain until Note OFF. Hold SEQ and turn PRESETS to shift the complete pattern one step per detent (left/right, wrapping, including HOLD links and locks); release SEQ to keep the current page. Long-press SEQ no longer opens the library; on SEQUENCER, SAVE opens the eight named sequence slots. PRESETS changes BPM, and ALGORITHM controls the independent metronome volume (0–100); ALGORITHM returns to note-sustain editing when recording ends. The screen playhead follows the audio step; LED timing stays independent of LCD drawing. Intermediate-brightness key LEDs use a faster 455 Hz scan (303 Hz with DIM LO) instead of the former 114/76 Hz pattern, reducing visible flicker while preserving the hardware scan driver and leaving the audio engine unchanged.
- **PROJECTS / STORAGE:** Save and manage user projects.
- **SETTINGS:** Master volume, autosave, startup project, storage and CRT/VCR appearance.

Use **HOME** to cycle the main oscillator pages. On any ordinary screen where SELECT has no dedicated function, turning **SELECT** opens the exact same screen list as on PLAY. Where SELECT already controls a step, FX selection, slot, scroll position or setting, that function remains unchanged. **SEL** confirms selections. Protected update, recovery and welcome screens are excluded from the global navigation shortcut. On pages with K1–K4 rotaries, the last turned knob is highlighted briefly with two small bars beside its circle. FX, FILTER, ENV, LFO, PLAY, SOUND SHAPE, SUB, ARP and SEQUENCER use the same indicator. In Channel EQ, the selected value row is marked instead; the fifth ALGORITHM knob on SUB shows the same focus when adjusting BLEND. User-defined projects use the existing RUPTURE format. The display has dedicated 1-bit animations for PLAY, LFO, ENV and SOUND SHAPE. Animations are rendered in the UI loop, outside the audio callback. On **ENV**, the wall-break dust and debris settle through eight short frames, while the broken wall continues to tremble subtly until Note OFF. On **LFO**, small debris keeps falling into the sinkhole in an eight-frame loop while a note is held; releasing the note closes the scene. PLAY and SOUND SHAPE retain their existing hold loops and shake.

## V1.81: Rapid-note realtime reliability

V1.81 targets rising **DMA LATE** and **GUARD** counts when playing rapid notes. The monophonic synth now avoids needless eight-voice rate and modulation housekeeping on note changes, and performance-gate countdowns no longer copy the full held-note stack on every PCM sample. The existing MIDI, panel-key, sustain, sequencer and ARP behavior is preserved. On a desktop host, long fast-note and sequence-gate PCM comparisons match V1.80 exactly; on-device cycle savings remain to be measured.

The LCD formerly treated every Note ON/OFF edge as an urgent full-screen transfer, even during dense playing. At rapid note rates V1.81 coalesces nonessential LCD frame updates to 75 ms, or 100 ms under recent audio pressure, with a final frame when activity stops. **No key, MIDI event, rendered audio sample or DSP parameter is delayed or dropped.** Explicit screen/menu controls retain priority. Visual feedback can become less frequent during very fast playing to protect the audio bus. The screensaver, UI assets and existing three reverb algorithms remain unchanged.

**Hardware validation is still required.** Restart your FM1, note the baseline DMA LATE, GUARD, DSP MAX and MIN SLACK, then repeat the fast-note stress that raised the counters in V1.80. The important result is that LATE and GUARD **do not increase**. Host PCM identity and cross-compilation cannot guarantee zero underruns on a real FM1. Keep a project backup and a verified Pico rescue option.

## Reverb engines and realtime safety (V1.80)

RUPTURE runs three distinct global **ROOM / HALL / SPRING** algorithms. ROOM uses a four-delay feedback-delay network with damping and decorrelated early arrivals; SPRING uses two cross-coupled delay paths and two dispersive allpass filters, including dispersion **inside** feedback. HALL retains its established diffuse reverb algorithm and V1.79 PCM character. These are algorithmic approximations rather than impulse responses of any specific physical room or spring tank. K1 selects the engine, K2 DECAY, K3 SIZE and K4 MIX. SIZE changes actual line lengths; at MIX 100 the output contains only the reverb return. The engines reuse the existing 4,096-sample reverb buffer plus existing 64-sample diffuser, with only a few bytes of additional control state. A new reverb algorithm may alter a preset's sound; save or back up projects before downgrading.

In the shared effects processor, active phaser, tremolo, auto-wah, tape and decimator Q15 processing now uses mathematically exact bounded 32-bit operations rather than unnecessarily expensive per-sample 64-bit arithmetic. The other synthesizer features, oscillators, SUB drift and HALL are unchanged. On a host, the full-HALL all-effects PCM output remains **byte-identical to V1.79**. The architecture does not reduce polyphony, FX availability or sample rate, but the more realistic ROOM network has extra memory reads/writes and may use more CPU than V1.79.

For diagnostics, go to **GLO → AUDIO DIAGNOSTICS**. `DSP MAX` is a historical maximum; `DMA LATE` and `GUARD` are cumulative. After reboot, verify that both counters stay **unchanged at zero** when actively playing both SUBs with fast DRIFT movements, maximum effects and rapid navigation. Even the user's observed 1/1 indicates a past deadline or guard event. Software crossbuilds cannot prove zero glitches or adequate device CPU margin. Do not consider this a hardware-certified studio/live release until full FM1 stress and listening tests pass. Retain a project backup and verified Pico recovery option.

## Project safety and editing

An asterisk in the screen header or next to the ACTIVE project indicates unsaved changes. The star disappears only when a project is successfully saved or a stored project is deliberately loaded; a failed automatic save does not remove it. Opening a project while unsaved changes exist requires another SEL confirmation; HOME cancels the pending action. The separate STORAGE > PROJECTS route uses the same warning.

A saved project is acknowledged only after the storage operation completes. Loading and saving show a distinct successful or failed result. A single **HOME + SEL** chord can undo the most recent K1–K4 knob gesture on the PLAY, SOUND SHAPE, SUB, TILT, FUZZ and SANDPAPER pages. Hold HOME, then tap SEL promptly. This is RAM-only. Other parameters, sequencer edits and project loads cannot be undone through this shortcut. Create a project backup before changing firmware.

## Firmware installation and recovery

The WebInstaller is available in `index.html` and `WebInstaller.html`. Use a supported desktop browser with Web MIDI SysEx permission, through HTTPS or localhost. The firmware package is `RUPTURE-V1.82.fwsc` and uses identity **FM-1_968**. Its protected bootheader and trusted OTA loader are retained from the prior working release.

The installer can show a confirmed transfer at 100% while the device is rebooting; confirmation that the new firmware is running requires reconnect/identity verification or confirmation on the FM1 itself. Keep the USB connection stable until the process completes. A firmware write is always a recovery-sensitive operation.

RUPTURE retains the native cross-firmware OTA handling and the official V15 return-to-stock verification path. Not every custom firmware is guaranteed compatible. For Windows-based recovery, use the **FM1 Recovery Windows** link on the website. Keep a working hardware rescue route, and back up before changing firmware.

## Source and building

- Native firmware: `vendor/etch-v1.88-source/firmware/fm1-native/`
- UI sprite sources: `assets/`
- Tests: `tests/`
- Native JieLi build (with a compatible toolchain):

```bash
python3 vendor/etch-v1.88-source/firmware/fm1-native/build.py --storage --updater --usb-audio-48k --multi-project
```

This project includes a **software-generated and audited** firmware package. Hardware testing of this release, including maximum-load audio stability, storage formatting and an actual V15 recovery transfer, remains necessary.

## Documentation

The [CHANGELOG](CHANGELOG.md) contains the release history. The [cheat sheet](CHEAT-SHEET.html) explains the synthesizer functions. Technical verification reports are in `docs/`. The README is intentionally a user guide, not a release log.

## Credits and license

**RUPTURE by SikkenWorks, Utrecht, the Netherlands.** Built on the FM1 open-firmware community and the included GPL-licensed source. See `LICENSE` and `licenses/` for the applicable licenses and acknowledgments.


## SYSTEM INFO and encoder controls

Open **GLO → SYSTEM INFO** to see firmware version, firmware identity, USB-link status, a coarse battery-voltage band (not an exact charge percentage), storage readiness, used project slots and DSP/DMA peaks. Select **SEL** for detailed audio diagnostics, or **HOME** to go back. The displayed DSP figure is a recorded peak, not a guaranteed live CPU benchmark.

The four knobs retain one unit of change per deliberate detent. Sustained fast turning may advance continuous values more quickly; discrete choices always follow the individual detents. The last adjusted knob gets a short focus marker. None of this changes the audio engine.

The PLAY, ENV, LFO and SOUND SHAPE graphics run on elapsed time, independently of the display refresh policy. Active sprite/shake changes are displayed on the next available UI pass, while idle redraws are suppressed. If a physical FM1 cannot render a full frame in time, visible frame drops are still possible; the audio path remains prioritized.

## Live sequencer recording

On SEQUENCER, **REC alone never starts the sequencer**. If the sequencer is **already playing**, pressing **REC** begins recording immediately, just as in previous releases. If the sequencer is **stopped**, press **REC** to arm without playing. Press **PLAY** next to start a **two-bar count-in** of eight quarter-note metronome clicks at the current BPM. The step grid and SEQ button LED pulse on each beat. The synth can still be played during the count-in, but those notes are **not recorded**. At the next bar boundary the sequencer and recording begin together on step 1, synchronized to the PCM clock. Press PLAY or REC again during the count-in to cancel it. There is no count-in if playback was already running. Notes played during REC are quantized to the **nearest sequencer step**, accounting for tempo, swing and the actual key-down contact time rather than delayed LCD updates. The first note played on the downbeat just after the two-bar count-in is captured on **step 1**; a press arriving in the last 50 ms before that downbeat is also eligible for step 1 when still held. A Note ON stores the quantized trigger, and keeping the key held automatically extends its recorded note duration over the following steps. Releasing it finishes the sustain. Other notes and parameter locks stay intact. If you turn **PRESETS while holding SEQ**, the entire sequence rotates by one step per encoder detent (counterclockwise = earlier, clockwise = later). Steps wrap at the pattern boundary and retain note lengths, explicit HOLD references and parameter locks. Without holding SEQ, PRESETS remains BPM. Long pressing SEQ does not open the library; SAVE on the sequencer opens the eight sequence slots. The same beat-aligned metronome sounds during the two-bar count-in and subsequent live recording at the project BPM, independently of the monophonic CXS/SAW voice. **PRESETS always controls BPM**, including during live recording. **While live recording is armed or active**, turn **ALGORITHM** to adjust the metronome volume from 0 (off) to 100. ALGORITHM keeps its normal functions (including sequencer note sustain) when recording is disarmed. Adjusting PRESETS changes the BPM for both the sequencer and its click; adjusting ALGORITHM changes only the click loudness. The click does not change synth volume, steal mono synth voices or sound during ordinary playback. The metronome volume is a session setting, not a per-project saved property.

With the transport **stopped**, select a sequencer step and press **SEL** to clear the **entire step**, including every note, local HOLD event and parameter lock. Directed HOLD events on later steps that depend on those deleted triggers are unlinked. Other notes and other steps remain untouched. Step clearing is blocked while playing or recording.

The ENV wall now lets airborne dust and falling fragments settle in eight exact 1-bit PNG frames, while a 1-pixel tremble remains throughout Note HOLD. Note OFF retains the original closing artwork.
