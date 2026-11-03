# RUPTURE

**A monophonic bass synthesizer for the M-VAVE FM1.**  
Cycle Exchange Synthesis (CXS), a selectable bandlimited SAW oscillator, two suboscillators, modulation, sound shaping and effects.

**SikkenWorks. Born out of passion.**

## Getting started

RUPTURE runs on the compatible M-VAVE FM1 hardware. Before installing custom firmware, back up your projects and make sure you have a verified recovery method. Keep the FM1 powered and connected throughout an OTA transfer. Do not interrupt the separate OTA loader while it is writing flash.

On first launch, an on-device welcome overlay walks through backup confirmation, an optional storage format, the CRT filter preference and **CONTINUE TO RUPTURE**. Answering NO to the backup question never prevents you from continuing. Formatting never starts automatically: select YES, then deliberately hold SAVE for three seconds to confirm a destructive initialization. If the storage already contains a valid RUPTURE project catalog, the wizard **does not erase existing projects**. Selecting NO skips formatting. CRT defaults to OFF unless a previous global preference exists. The wizard displays a clear, persistent formatting result: **FORMAT SUCCESSFUL**, **STORAGE ALREADY READY**, or **FORMAT FAILED**. Press SEL after reading the confirmation. The completion preference is stored with the existing device-global journal when storage is initialized; when storage is unavailable/foreign and formatting is skipped, completion cannot be saved and the wizard can reappear after reboot.

### Basic navigation

- **PLAY:** Main CXS/SAW oscillator. The four knobs control PRESSURE/BALANCE/MEMORY/DENSITY in CXS, and BRIGHT/EDGE/BODY/LEVEL in SAW.
- **SOUND SHAPE:** WEIGHT/EDGE/BITE/PUNCH. ALGORITHM switches the main oscillator. CXS WEIGHT boosts low-frequency body relative to the highs; CXS PUNCH also responds during sustained notes. Presets saved with nonzero WEIGHT/PUNCH sound different from V1.65.
- **SUB OSC 1 / SUB OSC 2:** Each sub has independent type, octave, detune, drift and blend.
- **FILTER:** Resonant filter controls.
- **ENV:** ADSR envelope controls.
- **LFO:** Synthesis modulation controls.
- **FX / EDIT:** Effects and channel EQ.
- **SEQUENCER / ARP:** Pattern and arpeggio features.
- **PROJECTS / STORAGE:** Save and manage user projects.
- **SETTINGS:** Master volume, autosave, startup project, storage and CRT/VCR appearance.

Use **HOME** to cycle the main oscillator pages, **SELECT** to browse settings/options and **SEL** to confirm. On pages with K1–K4 rotaries, the last turned knob is highlighted briefly with two small bars beside its circle. FX, FILTER, ENV, LFO, PLAY, SOUND SHAPE, SUB, ARP and SEQUENCER use the same indicator. In Channel EQ, the selected value row is marked instead; the fifth ALGORITHM knob on SUB shows the same focus when adjusting BLEND. User-defined projects use the existing RUPTURE format. The display has dedicated 1-bit animations for PLAY, LFO, ENV and SOUND SHAPE. Animations are rendered in the UI loop, outside the audio callback.

## Project safety and editing

An asterisk in the screen header or next to the ACTIVE project indicates unsaved changes. The star disappears only when a project is successfully saved or a stored project is deliberately loaded; a failed automatic save does not remove it. Opening a project while unsaved changes exist requires another SEL confirmation; HOME cancels the pending action. The separate STORAGE > PROJECTS route uses the same warning.

A saved project is acknowledged only after the storage operation completes. Loading and saving show a distinct successful or failed result. A single **HOME + SEL** chord can undo the most recent K1–K4 knob gesture on the PLAY, SOUND SHAPE, SUB, TILT, FUZZ and SANDPAPER pages. Hold HOME, then tap SEL promptly. This is RAM-only. Other parameters, sequencer edits and project loads cannot be undone through this shortcut. Create a project backup before changing firmware.

## Firmware installation and recovery

The WebInstaller is available in `index.html` and `WebInstaller.html`. Use a supported desktop browser with Web MIDI SysEx permission, through HTTPS or localhost. The firmware package is `RUPTURE-V1.67.fwsc` and uses identity **FM-1_968**. Its protected bootheader and trusted OTA loader are retained from the prior working release.

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
