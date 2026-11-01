# RUPTURE V1.26 build and test log

- Cross-target clang pi32v2 app build: PASS; app-sized image audited, entry 0x02000120.
- New editor safety regression: capture original input page to prevent one SAVE from both entering and committing a name.
- Both slot menus: SEL opens an occupied slot after a second SEL; SAVE overwrites a non-current occupied slot only after a second SAVE. Empty slot enters name input. SAVE+SEL retains rename.
- Sequence slot presence: stored name counts as occupied even for zero-step sequences.
- Battery: display width is completely filled at 100%, measured ADC full threshold lowered to 578 (hardware-specific calibration may still be needed).
- Browser: exact firmware-transfer progress from unique bytes served, never maximum requested offset.
- Device during OTA: progress represents actual native trusted-loader staging write, with clear STAGING OTA LOADER label. Full FM1 flash WRITE is performed by separate, unmodified trusted loader: its on-device percent cannot be read or displayed by RUPTURE. Browser displays USB transfer.
- Original OTA loader and 16KiB protected boot header: to be compared byte-for-byte in release packaging.
- Original project storage format preserved; application RAM/audio engine not deliberately altered.
- Tests: C UI battery/renderer, V1.26 source/regression, website embed/hash parity, native compilation.
- Hardware test of V1.26: NOT executed here; user reports repeatedly testing RUPTURE on physical FM1 units, but this exact new binary requires separate user verification.
- Full legacy regression run exceeded execution timeout during a slow storage test; earlier cases through V1.23 HELP passed. This is NOT a full regression PASS.
