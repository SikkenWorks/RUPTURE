# RUPTURE V1.84.1 — Startup Button LED / Speaker Pop Hotfix

## Scope

Baseline: **RUPTURE V1.84**. **Only the initial button LED current step is mitigated.**
All synth algorithms, ROOM/HALL/SPRING reverbs, master output code, hardware LED scan ISR, encoder handling, project layout, Welcome state machine and OTA are left intact. Display version and build metadata are updated to V1.84.1.

## Analysis / probable cause

The V1.84 function-LED policy uses `0x3ffc` (all twelve button LEDs brightly lit) when `ru_welcome.phase` is nonzero. That can abruptly increase the button LED load exactly when RUPTURE's Welcome UI appears. The original firmware has a different LED policy and might not generate the same power-rail current step. This is a **source-confirmed sudden load transition** but a **speaker-level causal diagnosis still requires physical measurement**.

Another independently known potential source is the binary PC6 analog-output enable edge. The code already delays PC6 until a period of silent DMA and USB stability; it is *not modified* in this hotfix. It may itself cause a speaker pop. Digital PCM fading does not necessarily suppress a purely analog bias transient.

## Implementation

- `rupture_hw_leds.h`: add a pure boot-only LED current-slew function. In Welcome, full-on button LEDs become dim except any physically held button. On cold boot the low-current dim masks are introduced one at a time at 20 ms intervals, and after ~260 ms the original normal LED policy is applied.
- `bringup.c`: apply this function in the existing main-loop LED refresh routine. No new interrupts, audio calls, GPIO pins, panel timers, changes to scan timing, or flash writes.
- `rupture_ui.c` / `build.py`: visible/build version string only.

## Verified checks

- Native JieLi `pi32v2` compile / application ABI validation: PASS, app 347,620 bytes.
- Deterministic rebuild: SHA-256 identical (see `build-v1841-repeat.log`).
- Previous V1.84 button/encoder test: PASS unchanged.
- New LED-slew host regression: PASS (no full-current Welcome group, one low-current LED per 20ms, physical press always immediate, normal LED policy identical after 260ms).
- All other native source files are hash-identical to V1.84, especially `audio_core.c`, `audio_target.c`, `native_fx.c`, `ota.c`, `hal/fm1_input.h`, `wave_store.c`.
- Original 16 KiB protected bootheader and OTA loader are byte-identical to V1.84.
- Firmware, WebInstaller and website package SHA-256 verified.

## Physical hardware validation still required

This release is a targeted **plop mitigation candidate**, **not a proven hardware fix**. On a real FM1, test with speaker volume low at first, then compare cold boots using V1.84 versus V1.84.1. If the plop persists at the same instant even after LED soft start, differentiate analog-enable PC6 from LED-current coupling. The existing GLO-at-power-on diagnostics keep PC6 muted; if no pop occurs with LEDs operating while PC6 is held low, investigate the analog enable signal, not the LEDs. No automatic firmware reflash is necessary for the diagnostic itself.

Keep a project backup and a verified hardware recovery option. No physical FM1 speaker capture, power-rail scope or PC6 probe was available in this container.
