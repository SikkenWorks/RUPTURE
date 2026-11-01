# RUPTURE V1.30: native firmware and regression report

- JieLi pi32v2 crossbuild and target app audit: PASS. App 209,664 bytes; 371,900 bytes application headroom.
- FWSC integrity/CRC/identity: PASS. 610,086 bytes, SHA-256 `57df3271fdcd9367111057c7448b09932b226a0579495b99b3d1395f6dd4cb77`, identity `FM-1_968`.
- Protected first 16 KiB and trusted OTA loader unchanged against bundled ETCH baseline: PASS in package assertions.
- Native 1-bit 240x240 screenshots exported from exactly the firmware renderer: PASS (53 screens).
- GLO fifth REPORT A BUG card, QR bitmap, settings alignment, meadow-to-canyon frame checks: PASS.
- QR code URL `https://github.com/SikkenWorks/RUPTURE/issues` decoded from native screenshot when isolated with adequate quiet zone; actual on-device phone camera scan is still to be tested.
- SETTINGS/STORAGE automatically refreshes catalog on entry: native routing/build and source contract PASS.
- Legacy ETCH sequence labels: dedicated true NOR emulator PASS. Covers: empty ETCH 24 excluded; deliberately named empty sequence included; actual note pattern named ETCH 26 retained when browsing a different project.
- Full historical regression suite through V1.28: PASS before final remote-sequence safeguard. Re-run of affected V1.28 NOR storage suite after safeguard: PASS. Dedicated V1.30 remote-sequence test: PASS. V1.29 and V1.30 UI/source tests: PASS.
- Standalone WebInstaller, GitHub Pages index and FWSC file hash/byte equality: PASS after final packaging.
- Existing DSP engines, effect parameters, project/sequence on-flash format, OTA loader and bootheader remain unchanged. Native read-only remote catalog logic and UI updated.
- **Physical FM1 test for V1.30: not carried out here.** Confirm live project catalog, sequences after reboot, phone scan on actual screen, and screensaver smoothness with MIDI/audio load before release.
