# RUPTURE V1.63 — Project Safety Verification

## Built artifact

- Compiler: supplied JieLi pi32v2 clang via `build.py --storage --updater --usb-audio-48k --multi-project`.
- Native application: 318,600 bytes; flash application headroom 262,964 bytes. Build and app entry checks PASS.
- `.fwsc`: 610,086 bytes; SHA-256 `fc1d8f444b043774830bd574f0909c5d4ecf118a3ee861fe6fc4b6f17e451b03`.
- Installer: same firmware embedded as loose file; website SHA/size validation PASS.

## Focused host regression results

- `test_v163_edit_safety.c`: PASS. Actual 240×240 native UI renders unsaved marker; project library and STORAGE show distinct unsaved-discard warnings; success and failure splash differ; undo gesture coalesces rapid turns.
- `test_v161_native_controls.c` / `test_v162_ui_labels.c`: PASS. Encoder behavior and system status pages retained.
- `test_v160_welcome.c` / `test_v159_welcome_storage.c`: PASS. Wizard paths, three-second format hold, storage initialization and CRC journal persistence/torn writes.
- `test_v156_panel_anim.c`, `test_v157_volcano_anim.c`, `test_v158_panel_shake.c`: PASS. PLAY/ENV/LFO/volcano animation state logic retained.
- `test_v155_cxs_pressure.c`: PASS. Native CXS PRESSURE response across 0..100, live sweeps and parameter extremes.
- `test_v143_ota_reclaim.c` using the new V1.63 `.fwsc`: PASS, including valid app-only OTA staging and failure-closed invalid-loader/CRC cases.
- Protected 16 KiB bootheader and whole trusted OTA loader: byte-identical to V1.62. 13 critical audio/ISR/OTA/storage source files byte-identical to V1.62.
- Website `tools/build_site.py`, `tools/test_site.py`, `tests/test_website_release.py` and release cleanup validator: PASS.

## Release behavior

- `*` marks unsaved edits, independently of automatic-save retry gating.
- Existing project OPEN confirmation now explicitly warns before discarding unsaved RAM changes. STORAGE > PROJECTS uses the same explicit confirmation, including reload of the current project.
- Save/load results appear only after the native storage operation returns success/failure. A failed automatic save keeps the unsaved indicator visible without repeatedly writing flash.
- HOME held plus SCL tap undoes the previous simple knob gesture on PLAY/SHAPE/SUB/TILT/FUZZ/SANDPAPER; sequencer, composite FX and project-load undo are intentionally out of scope.
- No project format change, new automatic flashwrite, extra audio buffers or extra DSP code.

## Not performed

- The full historical regression suite was **not** run for this release; the focused tests above passed.
- No physical FM1 audio/DMA benchmark, battery test, project save/load/undo hardware test, USB hotplug test, cross-firmware handoff or actual stock V15 restoration.
- Host OTA checks prove code-path and package continuity, **not** guaranteed safe cross-firmware switching on every FM1/third-party firmware.
