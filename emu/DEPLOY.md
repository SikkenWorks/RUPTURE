# RUPTURE FM-1 Emulator V2.4

This package contains the **complete source project**. Publish it with the included GitHub Actions workflow to compile the WebAssembly engine and generate a deployable website.

## Edition

**RUPTURE edition:** This site bundles exactly `web/RUPTURE.fwsc` and launches that package by default. Firmware uploading, drag/drop firmware replacement, remote firmware catalogs, URL-based switching, and importing saves from other firmwares are disabled. Browser autoplay restrictions may require clicking **Start RUPTURE** to enable sound. The exact bundled package is SHA-256 pinned.

All standard emulator controls are preserved: fullscreen/touch panel, keyboard shortcut editor, CPU MHz, performance/accurate mode (with built WASM), audio response settings, MIDI, flash saves and diagnostics.

## Publish on GitHub Pages

1. Extract this ZIP and push its **contents** to a GitHub repository root (including the hidden `.github` folder).
2. In GitHub Settings > Pages > Build and deployment, select **GitHub Actions**.
3. The workflow tests the code, builds `fm1_web.wasm` from Rust source and deploys the static site. It also validates the embedded firmware SHA-256 for RUPTURE.
4. Use the URL supplied by GitHub Pages. HTTPS is required for Web MIDI and the PWA (localhost also works for development).

A static upload of `web/` alone is **not** enough until `fm1_web.wasm` is built using `bash web/build.sh <output-directory>` or downloaded from the resulting GitHub Pages artifact. This environment did not compile Rust/WebAssembly, so the GitHub workflow must complete successfully before the full hardware-accurate mode can be claimed working.

## Credits and license

- **Original FM1 emulator by Joakim Eriksson:** https://github.com/joakimeriksson/fm1-emu
- Original emulator source and derivative source retain the included **MIT LICENSE**.
- Bundled RUPTURE firmware by SikkenWorks: https://github.com/SikkenWorks/RUPTURE . The emulator MIT license does **not** automatically relicense this separate firmware file.

All emulator source files, tests and licenses are present in this archive. For third-party components and historical documentation, see `UPSTREAM_README.md` and `README.md`.
