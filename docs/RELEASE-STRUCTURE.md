# RUPTURE Release Folder Policy

- Root files: the active firmware `.fwsc`, `index.html`, `WebInstaller.html`, `README.md`, `CHANGELOG.md`, `CHEAT-SHEET.html`, manifest and deployment documentation.
- README: evergreen product documentation and usage only. Do not duplicate the CHANGELOG.
- CHANGELOG: all release-specific change notes and fixes.
- `vendor/etch-v1.88-source/`: complete buildable native source and OTA implementation.
- `native-build/`: the single current compiled application, audit report and build options.
- `reference/`: minimal trusted firmware references for bootheader and loader parity. The only current version at the root is the new `.fwsc`.
- `assets/`: source pixel art, boot assets and first-run UI previews.
- `tests/`: software tests for controls, storage, animation and firmware OTA.
- `docs/`: verification and non-narrative technical support documents.
- `tools/`: deterministic build/package and verification scripts.

Never substitute a preview image for a firmware build. Verify bootloader hashes and avoid adding new storage partitions. Physical FM1 testing is required before broad release.
