# RUPTURE website / GitHub Pages

`index.html` is the official RUPTURE information page and WebInstaller.
It embeds exactly the firmware in `RUPTURE-V1.89.fwsc` and must not be mixed with a different version.

## Publish to GitHub Pages

1. Extract the `RUPTURE-V1.89-GITHUB-PAGES.zip` contents to the **root** of the GitHub Pages repository, keeping the directories intact, **including the hidden `.nojekyll` file**. `.nojekyll` is needed so GitHub Pages serves `README.md` and `CHANGELOG.md` as raw Markdown rather than rebuilding them as Jekyll pages. (Alternatively, place all contents inside `/docs` and choose `/docs` as the Pages source.)
2. In GitHub, go to **Settings → Pages**, choose **Deploy from a branch**, the correct branch and **/(root)** (or **/docs**), and save.
3. Open your HTTPS Pages URL. `index.html` is the homepage instead of `README.md`. The page automatically fetches `README.md`, `CHANGELOG.md` and `CHEAT-SHEET.html` from the same directory.
4. For firmware installation, use **desktop Chrome or Edge** with Web MIDI SysEx access. HTTPS or `localhost` is required. Do not flash from an untrusted mirror.

## Update for every new RUPTURE firmware version

1. Update `RELEASE-MANIFEST.json` with the new firmware filename, version, exact byte size and SHA-256.
2. Put the new `.fwsc` in the same folder and replace `WebInstaller.html` with that release's independently verified installer.
3. Update `README.md` as an evergreen guide only, `CHANGELOG.md` as the exclusive release history, and `CHEAT-SHEET.html` as needed. The new version should lead the CHANGELOG. Do not copy release-note text into README.
4. Optionally replace `assets/boot-animation.gif` and `assets/boot-final.png` if the artwork changes.
5. Run `python3 tools/build_site.py` from the release root to regenerate `index.html`, then `python3 tools/test_site.py` for integrity checks.
6. Commit/publish `index.html`, the `.fwsc`, source documents and any assets together. **Never carry over an older embedded firmware package to a new version.**

On GitHub Pages, Markdown/cheat updates are loaded automatically on refresh, even without rebuilding `index.html`. However, every firmware update needs a fresh `index.html` build to embed and verify the matching package. If opened with `file://` or if a Markdown fetch fails, the single-file page uses the build-time embedded documentation snapshots. The firmware is always embedded and does not depend on network access.

## Safety and credits

This website does not modify the protected FM1 bootheader, trusted Felucca-derived OTA loader, or any native C source. The original two-stage OTA CHECK → trusted loader WRITE protocol is retained. Software testing of the release package cannot establish safe operation on every physical FM1. Back up projects and confirm a hardware recovery path before updating.

See `README.md`, `LICENSE` and `licenses/FELUCCA-LICENSING.txt` for copyright and upstream credits.

## Release hygiene

Each RUPTURE release must have exactly one current `.fwsc` at the project root; earlier firmware belongs in `reference/` if needed for byte audits. Remove old logs, compiled scratch and redundant builds before packaging. Keep source, tests, license and OTA audit references. Always verify the new embedded firmware and SHA-256 against the standalone `.fwsc`.
