# RUPTURE V1.62 — Native Experience Test Report

## New features
- GLO > SYSTEM INFO shows actual firmware identity, USB link, battery state, flash storage status, occupied project slots, recorded DSP peak and late DMA count. SEL opens existing AUDIO DIAGNOSTICS. The page is read-only.
- Continuous knobs use one detent per slow step, with bounded two-step adjustment on sustained fast turns. Discrete TYPE/OCT/MODE/selection controls remain one physical detent per option. A focus marker briefly highlights the last knob.
- Existing 240x240 layouts, CRT default OFF, first-run wizard, animations, storage workflow and installer functions retained.

## Tests actually executed
- Native JieLi pi32v2 build with --storage --updater --usb-audio-48k --multi-project: PASS
- App audit: PASS, 316,996 bytes of 581,564-byte app slot.
- Baseline reproducibility: V1.60 app reassembled to exactly the same original flash bytes: PASS.
- New .fwsc integrity, unchanged first 16 KiB boothead and OTA loader, preserved FM-1_968 identity: PASS.
- New .fwsc native OTA test_v143_ota_reclaim: PASS including valid update, residual loader, unknown staging, CRC and modified boot guard failure.
- 14 original audio/OTA/storage source files byte-identical to V1.60: PASS.
- Encoder and native SYSTEM INFO/GLO/focus tests: PASS.
- Welcome YES/NO and 3-second format confirmation host tests: PASS.
- Existing V1.59 storage journal/welcome metadata host test: PASS.
- Existing V1.55 sustained CXS PRESSURE host DSP regression: PASS.
- Native 240x240 UI screenshots exported from actual C renderer: PASS.
- Site, embedded .fwsc hash/length, README/CHANGELOG fallback, right-click/selection guards: PASS.

## Not validated
- No physical FM1 verification of encoder feel, CPU headroom, live USB and battery readouts.
- Cross-firmware transition and official V15 stock restore NOT newly run on physical hardware; stock V15 fixture not included.
- Full historical regression suite not run.
- Do not claim flash/install success from a package build alone. Keep a known recovery route and back up existing projects.
