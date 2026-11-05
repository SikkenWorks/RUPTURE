# RUPTURE FM-1 Emulator V2.4

**Original emulator by Joakim Eriksson** · https://github.com/joakimeriksson/fm1-emu  
**RUPTURE firmware by SikkenWorks** · https://github.com/SikkenWorks/RUPTURE  

This is the complete source code and GitHub Pages deployment project for a full-screen, mobile-friendly M-VAVE FM-1 emulator, including a Progressive Web App (PWA). The original emulator's MIT license and source attribution are preserved in `LICENSE`.

## This edition

**Dedicated RUPTURE edition:** The supplied `RUPTURE.fwsc` is included as `web/RUPTURE.fwsc` and automatically selected/validated when the site opens. Click **Start RUPTURE** to unlock browser audio if needed. No firmware upload/switch controls, drop-to-load FWSC, URL-based firmware selection, or backups from a different firmware are accepted. A SHA-256 hash pins the package to the exact uploaded bytes. Import/export of matching RUPTURE saves remains available.

SHA-256 of embedded `RUPTURE.fwsc`:

`dd0988abeefd5439db602d8a8420e92954de313bf4e041a7f1495144e0603993`

This is **not a firmware flashing tool**: it runs the binary in the browser emulator and never changes a connected physical FM1.

## Main features

- Fullscreen touch/mouse FM1 front panel and responsive mobile browser/PWA interface
- Keyboard and remappable shortcuts (48 assignments)
- CPU clock (MHz), Performance/Hardware Accuracy modes, audio response preset
- Save/Export save/Import save, browser local flash storage
- Web MIDI, diagnostics and WAV comparison with a physical FM1 recording
- Audio resampling for browsers using 48 kHz while FM1 emulates a 44.1 kHz source
- Favicon and installable PWA icons
- Original code attribution to Joakim Eriksson, plus full MIT license

## Deploy on GitHub Pages

1. Extract **this complete ZIP** and upload the contents (including `.github/`) to the root of a repository. Avoid nesting everything inside an extra folder.
2. Go to GitHub **Settings → Pages → Build and deployment** and choose **GitHub Actions**.
3. Push or run the included `Build and deploy FM-1 Emulator PWA` workflow manually under Actions. It runs regression tests, compiles the Rust emulator core to WebAssembly, verifies the site and publishes it to Pages.
4. GitHub Actions also uploads a **website artifact ZIP** containing the ready-to-upload `index.html`, `fm1_web.wasm`, scripts, icons and PWA files. Extract this artifact to your own HTTPS site if desired.

**Important:** The source ZIP does not include a compiled `fm1_web.wasm`. A plain upload of the source or just `web/` will not provide a fully self-contained V2.4 emulator. Let the GitHub Action finish first, or build locally:

```bash
rustup target add wasm32-unknown-unknown
bash web/build.sh _site
```

The ready-to-upload site is then in `_site/`. Browser audio may require a Start tap, and Web MIDI needs a supported browser and HTTPS.

## Tests and boundaries

- Run `node --test web/*.test.mjs` and `python3 -m unittest discover -s tools/tests -v`.
- Tests for the browser frontend, firmware lock, save management, audio conversion and FWSC parsing are included.
- Rust/WebAssembly compilation and live FM1 equivalence are not claimed to have been completed in this environment; the GitHub workflow builds the engine and physical hardware validation is a separate step.

## Credits and license

**Joakim Eriksson created the original emulator and reverse-engineering implementation.** Original project: https://github.com/joakimeriksson/fm1-emu . This derivative retains his copyright notice and MIT license. His original documentation is available at `UPSTREAM_README.md`. This project is not affiliated with M-VAVE.

**Firmware ownership is separate:** The included `RUPTURE.fwsc` was provided by the project owner for this edition. RUPTURE source/repository: https://github.com/SikkenWorks/RUPTURE . The emulator MIT license does not automatically grant a license to redistribute unrelated firmware files.

## Files

- `web/`: web app and PWA files
- `web/firmware-policy.js`: firmware-selection and backup-import policy
- `web/build.sh`: standalone website builder, using the Rust WebAssembly target
- `.github/workflows/pages.yml`: tests, build, website artifact and Pages deployment
- `crates/`: full emulator core/source from the previous version
- `LICENSE`: original emulator MIT license
- `CHANGELOG.md`: version history
- `DEPLOY.md`: short publishing instructions
- `docs/README-V2.3.md`: archived V2.3 development notes
