# RUPTURE FM-1 Emulator V2.4 website

This is the **built GitHub Pages PWA**. The original emulator was created by **Joakim Eriksson**: https://github.com/joakimeriksson/fm1-emu (MIT license, see `LICENSE`).

RUPTURE firmware is bundled as `RUPTURE.fwsc`. It is selected automatically and cannot be replaced in the app. Press **Start RUPTURE** if the browser requires a tap for audio. RUPTURE by SikkenWorks: https://github.com/SikkenWorks/RUPTURE .

Use HTTPS or localhost for installing the PWA and access to supported Web MIDI features. Use the toolbar to adjust clock, emulation mode, response, MIDI and shortcuts, and to save/import/export backups. Browser storage is local and may be cleared when you clear site data.

This website distribution requires `fm1_web.wasm` next to `index.html`. It is built by the accompanying source repository's GitHub Action or with `bash web/build.sh _site`.
