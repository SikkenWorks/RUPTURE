# RUPTURE

**A monophonic bass synthesizer for the M-VAVE FM1.**  
Cycle Exchange Synthesis (CXS), a selectable bandlimited SAW oscillator, two suboscillators, modulation, sound shaping and effects.

**SikkenWorks. Born out of passion.**

## Getting started

RUPTURE runs on the compatible M-VAVE FM1 hardware. Before installing custom firmware, back up your projects and make sure you have a verified recovery method. Keep the FM1 powered and connected throughout an OTA transfer. Do not interrupt the separate OTA loader while it is writing flash.

On first launch, an on-device welcome overlay walks through backup confirmation, an optional storage format, the CRT filter preference and **CONTINUE TO RUPTURE**. Answering NO to the backup question never prevents you from continuing. Formatting never starts automatically: select YES, then deliberately hold SAVE for three seconds to confirm a destructive initialization. If the storage already contains a valid RUPTURE project catalog, the wizard **does not erase existing projects**. Selecting NO skips formatting. CRT defaults to OFF unless a previous global preference exists. The setup completion preference is stored with the existing device-global journal when storage is initialized; when storage is unavailable/foreign and formatting is skipped, completion cannot be saved and the wizard can reappear after reboot.

### Basic navigation

- **PLAY:** Main CXS/SAW oscillator. The four knobs control PRESSURE/BALANCE/MEMORY/DENSITY in CXS, and BRIGHT/EDGE/BODY/LEVEL in SAW.
- **SOUND SHAPE:** WEIGHT/EDGE/BITE/PUNCH. ALGORITHM switches the main oscillator.
- **SUB OSC 1 / SUB OSC 2:** Each sub has independent type, octave, detune, drift and blend.
- **FILTER:** Resonant filter controls.
- **ENV:** ADSR envelope controls.
- **LFO:** Synthesis modulation controls.
- **FX / EDIT:** Effects and channel EQ.
- **SEQUENCER / ARP:** Pattern and arpeggio features.
- **PROJECTS / STORAGE:** Save and manage user projects.
- **SETTINGS:** Master volume, autosave, startup project, storage and CRT/VCR appearance.

Use **HOME** to cycle the main oscillator pages, **SELECT** to browse settings/options and **SEL** to confirm. User-defined projects use the existing RUPTURE format. The display has dedicated 1-bit animations for PLAY, LFO, ENV and SOUND SHAPE. Animations are rendered in the UI loop, outside the audio callback.

## Firmware installation and recovery

The WebInstaller is available in `index.html` and `WebInstaller.html`. Use a supported desktop browser with Web MIDI SysEx permission, through HTTPS or localhost. The firmware package is `RUPTURE-V1.59.fwsc` and uses identity **FM-1_968**. Its protected bootheader and trusted OTA loader are retained from the prior working release.

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
