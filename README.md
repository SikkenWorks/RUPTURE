# RUPTURE for M-VAVE FM1

RUPTURE is a monophonic bass synthesizer by SikkenWorks. It pairs Cycle Exchange Synthesis (CXS) and GROWL with two sub oscillators, an eight-slot effects chain, an arpeggiator, and a step sequencer.

## Install

1. Back up your FM1 data and keep a known working firmware and recovery method available.
2. Extract the full project ZIP. Open `WebInstaller.html` in a desktop browser with Web MIDI and SysEx support.
3. Connect the FM1 by USB and follow the installer prompts. Keep the device powered and connected until the update and reboot are complete.

| Firmware file | Audio processing |
| --- | --- |
| `RUPTURE-V2.0.61-STABLE.fwsc` | Single-core audio. This is the default installer choice. |
| `RUPTURE-V2.0.61-CORE2-EXPERIMENTAL.fwsc` | Experimental second-core effects processing. Use this build if you have a working recovery method. |

Both builds have passed software checks. Startup audio at the physical jack still needs confirmation on an FM1.

## First launch

Follow the welcome prompts and choose whether to run the Quick Tour. If the device offers to format storage, back up the data first and confirm formatting only if you intend to erase it. You can revisit the tour from **GLO → QUICK TOUR**.

## Play and shape sounds

| Control | Function |
| --- | --- |
| **HOME** | Return to the main playing screen. |
| **K1–K4** | Adjust the parameters labeled on the current screen. |
| **FX** | Open the effects chain. Hold **FX** to open **BEND**. |
| **PRESETS** | Adjust the assigned performance parameter. The default is BPM. |
| **SEQ** | Open the step sequencer. |
| **GLO** | Open settings, system information, and other utilities. |

Build a sound from CXS or GROWL, blend in the sub oscillators, and shape it with the filter, envelope, LFO, and effects. The effects chain lets you reorder slots and use multiple instances of the same effect with independent settings.

## Record and save

- To record into a stopped sequence, press **REC** to arm it, then press **PLAY** to start a two-bar count-in. During playback, **REC** starts recording immediately.
- Hold **SEQ** and turn **PRESETS** to rotate the pattern one step per detent.
- Press **SAVE** on the sequencer page to open its sequence slots. From regular sound pages, **SAVE** opens the project workflow.
- Use **PROJECTS** to save and load named projects. RUPTURE supports up to 12 project slots and eight sequence slots per project, subject to available storage.
- Flash operations can briefly mute audio. Wait for the save confirmation before switching off the FM1.

## Connections and diagnostics

Use the 3.5 mm audio output for analog monitoring or USB audio for digital capture. External MIDI can arrive over USB or through the TRS MIDI input. Open **GLO → SYSTEM INFO** and press **SEL** to view audio diagnostics, including DMA deadline misses.

## Build from source

Set `JIELI_TOOLCHAIN` to the path of a compatible JieLi toolchain and run the following command from the project root.

```sh
python3 vendor/etch-v1.88-source/firmware/fm1-native/build.py --storage --updater --usb-audio-48k
```

For the experimental CORE2 build, add `--dual-core-lab --dual-core-fx-live-lab`. Native firmware sources are in `vendor/etch-v1.88-source/firmware/fm1-native/`, and the host checks are in `tests/`. See [the control cheat sheet](CHEAT-SHEET.html) for more controls.

## Credits and license

| Credit | Contribution |
| --- | --- |
| **Joppe Sikken / SikkenWorks** | RUPTURE concept, sound design, CXS, interface, integration and development in Utrecht, the Netherlands. ETCH supplies the FM1 source base adapted for RUPTURE. [SikkenWorks](https://github.com/SikkenWorks) · [ETCH](https://github.com/SikkenWorks/ETCH) |
| **Leo Kuroshita (@kurogedelic) / Hügelton Instruments** | Felucca provides the FM1 foundation used through ETCH, including startup, hardware drivers, interrupts, LCD, MIDI and OTA/recovery infrastructure. GPL-3.0-only. [Felucca](https://github.com/hugelton/Felucca) |
| **M-VAVE** | Original FM1 hardware and stock firmware. RUPTURE is independent firmware for the FM1. |
| **JieLi Technology** | AC79-series SDK, toolchain and boot/configuration components. See the applicable SDK and toolchain notices. [AC79 SDK](https://gitee.com/Jieli-Tech/fw-AC79_AIoT_SDK) |
| **SikkenWorks B1 and BEETLE** | B1 V4.3 HF2 is a reference for installer reconnect handling in the inherited ETCH source. BEETLE is a reference for the battery indicator UI. |

The bundled upstream notices record the following provenance for Felucca and its components.

| Credit | Upstream contribution |
| --- | --- |
| **charlesvestal / X0X** | Browser emulator design reference. GPL-3.0. [X0X](https://github.com/charlesvestal/fm1-x0x) |
| **Hügelton Instruments / CrispyZebra** | PHASE oscillator waveforms. GPL-3.0. [CrispyZebra](https://github.com/hugelton/CrispyZebra) |
| **Hügelton Instruments** | Felucca drum voices and kit, Hügelton Sample Pack and controls artwork. GPL-3.0-only. |
| **Electrosmith, Corp and Emilie Gillet / DaisySP** | Physical-model DSP. MIT. [DaisySP](https://github.com/electro-smith/DaisySP) |
| **Emilie Gillet / Rings** | Sympathetic string DSP. MIT. [Rings](https://github.com/pichenettes/eurorack) |
| **Google Inc. and Pascal Gauthier / msfa** | FM synthesis core from Dexed. Apache-2.0 for msfa. [Dexed](https://github.com/asb2m10/dexed) |
| **Hügelton Instruments / Fukiai** | Icon font. MIT. [Fukiai](https://github.com/hugelton/Fukiai) |
| **The Inter Project Authors / Inter Tight** | UI typeface. SIL OFL 1.1. [Inter Tight](https://github.com/rsms/inter-tight) |
| **The DotGothic16 Project Authors** | Browser emulator typeface. SIL OFL 1.1. [DotGothic16](https://github.com/fontworks-fonts/DotGothic16) |
| **Versilian Studios / VSCO-2 Community Edition and VCSL** | Instrument sample provenance. CC0 1.0. |
| **Tony Gies / klattsch, Dennis Klatt and Hillenbrand et al.** | Voice synthesis design and formant data references. [klattsch](https://github.com/tgies/klattsch) |

See [LICENSE](LICENSE), [Felucca’s licensing inventory](licenses/FELUCCA-LICENSING.txt) and the [bundled license texts](vendor/etch-v1.88-source/firmware/fm1-native/LICENSES/) for the full terms and copyright notices.