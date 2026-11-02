# RUPTURE V1.50 — Build and test record

- Actual FM1 crossbuild: **PASS** using JieLi pi32v2 toolchain. App image **253,380 bytes**, budget **581,564 bytes**, headroom **328,184 bytes**.
- Installer .fwsc: **PASS** package structure, CRC, app CRC and product identity `FM-1_968`.
- Firmware SHA-256: `21bfe57cce5f873eca7daacb6b73d3f92795e2d895019e49387a31f2351ef17d`.
- Old V1.49 protected bootheader (first 16 KiB): **byte-identical**.
- Old V1.49 OTA loader: **byte-identical**.
- Native OTA app-only replay: **PASS** all reported checks (see host test output).
- Original saver 64-frame forward, 2.2-second hold, reverse/back-to-zero: **PASS** existing V1.45 state machine test.
- Native C saver frame 0 matches original upper city art; frame 63 tower differs due to visible split: **PASS**.
- Native C saver frames 60 and 63: **identical**, verifying stable peak hold.
- Native C screenshot export: **PASS**. Preview GIF is built from the native C frame renderer, not redrawn.
- WebInstaller embedded firmware, SHA and length: **PASS**.
- Full historical `tests/run_tests.sh`: **NOT COMPLETED** (execution timed out after tests through V1.22 passed). Not presented as full-suite success.
- Physical FM1 boot/install/display/audio timing: **NOT TESTED**. Confirm recovery path before flashing.

The original audio, sequencer, project format, DSP, and OTA source files were not modified for the V1.50 screensaver change. The new code lives in `vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c`.
