# RUPTURE V1.84.2 — LED / PC6 speaker-pop mitigation audit

## Scope

Reference: V1.84.1. This release is limited to (1) function-button LED behavior during protected/audio-fault screens and (2) separation of LED-current transitions from the **existing** PC6 analog-enable edge. No changes to the sound engine, synth parameter DSP, three reverbs, ADC, sequencer, storage, OTA C module or hardware LED scan ISR. Hardware behavior remains unverified.

## Source-confirmed failure modes

- The previous protected-screen LED policy returned `solid=0x3ffc`, switching **all twelve** function LEDs to full current. In audio-fault state, the realtime fault stop also lowers PC6. Those events can coincide and generate an abrupt supply/current transient. This is source-confirmed, but electrical coupling to the speaker has not been measured.
- The previous PC6-enable gate already required silent DMA, time for the codec and USB stability, but did **not** require the button LED bank to have reached a stable load before enabling the analog output. A coincident UI/LED transition remained possible.
- PC6 itself is a binary analog power control; this firmware does **not** PWM PC6 or assert that it eliminates an intrinsic analog amplifier bias pop.

## Changes

- `rupture_hw_leds.h`: protected screens (AUDIO STOP/SAFE/UPDATE/Welcome) now have only fast, low-current dim background button LEDs, with immediate full-bright feedback on physically pressed buttons. Normal transport/record/SEQ LED policy unchanged.
- `native_output_startup.h`: host-testable 400 ms LED-bank quiet requirement before the existing analog-enable edge.
- `bringup.c`: track actual post-soft-start button LED masks only in the existing UI thread; prime them before starting the audio link; keep the Welcome LED ramp updating while its wizard owns navigation.
- `audio_target.c`: apply the LED quiet qualification **only before** the initial PC6-on edge; keep old mute, watchdog/underrun stop, USB timing, PCM fade and every audio render path unchanged.
- `rupture_ui.c` and `build.py`: version metadata updated to V1.84.2 only.

## Validation

| Test | Outcome |
|---|---|
| Native JieLi pi32v2 cross-build and application ABI audit | PASS |
| Deterministic rebuild SHA-256 | PASS |
| Protected LED current, physical key priority, normal LED status | PASS host C tests |
| PC6 quiet gate before/after 400 ms and TIMER4 wrap | PASS host C test |
| Existing V1.84 hardware/encoder regression | PASS host C test |
| Firmware UFW, app directory and CRC verifier | PASS |
| Firmware/WebInstaller/website embedded bytes and SHA-256 | PASS |
| Protected first 16 KiB flash header | EXACTLY IDENTICAL to V1.84.1 |
| Trusted OTA loader 6,930 bytes | EXACTLY IDENTICAL to V1.84.1 |
| Unchanged other native code, including audio_core.c, native_fx.c, ota.c, project storage, hardware LED scan ISR | SOURCE HASH PARITY |
| Physical FM1 speaker pop / PC6 pin / rail voltage | NOT TESTED |

**App size:** 347,688 bytes, with **233,876 bytes flash application headroom**. This is not a CPU/DMA timing measurement.

**Firmware SHA-256:** `dd0988abeefd5439db602d8a8420e92954de313bf4e041a7f1495144e0603993`

**App SHA-256:** `868e09b6beee56b0e6bf90c87b9d01cdd104c96c0e23ad950e405eb37723768b`

**Protected header SHA-256:** `a65e0e1a2fad1db943c00b529b45553c524257146145db61b69628cefcf2b2c9`

**OTA loader SHA-256:** `d71d1b7c12adede951605e758a3580dacccedc427d49e1fceaf51decccc68a4b`

## Physical acceptance

With a project backup and proven independent recovery option, begin at low speaker volume. On V1.84.2 test a cold power-up, normal restart, Welcome wizard, and a forced/observable AUDIO STOP event only if it occurs naturally during normal testing (do not deliberately induce corrupt DMA). Compare behavior with V1.84.1. The existing **GLO held at power-on** diagnostic keeps PC6 off during normal codec initialization. If the plop disappears only when PC6 is held off, that implicates the powered analog chain but does not distinguish PC6's own bias transient from LED coupling into a powered amplifier. Hardware measurement at PC6 and the amplifier rails is needed to settle that distinction.

The root cause of any remaining hardware-intrinsic pop **cannot be certified fixed** from cross-compilation alone. Do not try PC6 PWM or arbitrary pin-mux changes without electrical validation.
