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

| Credit | Contribution to RUPTURE |
| --- | --- |
| **Joppe Sikken / SikkenWorks** | RUPTURE concept, sound design, CXS, interface and development in Utrecht, the Netherlands. RUPTURE adapts the ETCH FM1 source base. [SikkenWorks](https://github.com/SikkenWorks) · [ETCH](https://github.com/SikkenWorks/ETCH) |
| **Leo Kuroshita (@kurogedelic) / Hügelton Instruments** | Felucca provides underlying FM1 startup, hardware drivers, interrupts, LCD, USB MIDI/audio and OTA infrastructure carried through ETCH. GPL-3.0-only. [Felucca](https://github.com/hugelton/Felucca) |
| **Juan Ignacio Antonucci / JIANT FM1, Melodee and charlesvestal / X0X** | The experimental CORE2 build adapts JIANT's second-core startup and exception handling. The source headers trace that work back to Melodee and X0X. GPL-3.0. [X0X](https://github.com/charlesvestal/fm1-x0x) |
| **SikkenWorks B1 and BEETLE** | B1 V4.3 HF2 contributes updater reconnect handling in the WebInstaller. BEETLE informed the battery indicator design. |
| **The Firmware Hub** | The OTA identity reply follows its convention for reporting the flash chip's JEDEC ID. |
| **M-VAVE** | Original FM1 hardware and stock firmware. RUPTURE is independent firmware for the FM1. |
| **JieLi Technology** | AC79-series SDK and pi32v2 toolchain. RUPTURE's release packaging preserves boot and configuration components from its trusted firmware base. [AC79 SDK](https://gitee.com/Jieli-Tech/fw-AC79_AIoT_SDK) |

See [LICENSE](LICENSE) for project licensing and the [bundled license texts](vendor/etch-v1.88-source/firmware/fm1-native/LICENSES/) for third-party notices. The [Felucca licensing inventory](licenses/FELUCCA-LICENSING.txt) records the broader upstream history.

<!-- RUPTURE release documentation rule: For every iteration, including when resuming in a new chat, carry forward the current concise product README. Keep version history in CHANGELOG.md. Verify that the Credits section retains all documented contributors, upstream projects and license notices before delivering the next README. -->
