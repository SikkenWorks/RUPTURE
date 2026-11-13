#!/usr/bin/env python3
"""Update V1.88 release metadata + docs without touching the V1.87 OTA engine."""
from pathlib import Path
R=Path(__file__).resolve().parents[1]
change=R/'CHANGELOG.md'
s=change.read_text()
new='''# RUPTURE V1.88 — Five Curve-Linked BEND Animations

## V1.88 (2026-11-11)

- **K3 selects one unique 8-frame 224×30 1-bit animation per curve**: LINEAR = unchanged V1.87 Mount Everest; SOFT = monolith smashing through stone; HARD = a three-masted pirate flagship bursting up from a meadow; S-CURVE = a steady close-up camera travelling up the flagship, from cannon-pierced hull to Jolly Roger; EXP = a colossal Kraken tearing open the ground.
- The 40 pre-rendered 1-bit frames are generated **offline** and packed into constant 33,600-byte flash ROM data. No additional audio ISR work, realtime procedural art, animation framebuffer, voices or DSP effects. The original 5 pre-FX K3 response curves remain byte-identical.
- **UI-only shake** on all sequences except S-CURVE. Up to 4 tiny ±1/2-pixel offsets cycle at 85ms while actively bending. The existing audio-first LCD refresh budget remains authoritative and can defer frames if audio is busy.
- V1.87 Everest sprites are kept bit-for-bit, including their original height and 1-bit monochrome appearance.
- Existing **OTA/Connection Fix** (NUL-safe identity parsing, generic Windows MIDI ports), both supported flashchip JEDEC IDs (Puya `0x856014`, UC25HQ80 `0xB36014`), trusted OTA loader, 16KiB protected boothead and flash layout remain untouched.
- Audio DSP, all 3 reverbs, presets, sequencer, MIDI queue, PC6 speaker gate and save format remain unchanged. Hardware verification required before unrestricted release.

---

'''
if not s.startswith('# RUPTURE V1.88'):change.write_text(new+s)
p=R/'README.md';s=p.read_text()
assert s.startswith('# RUPTURE V1.87')
s=s.replace('# RUPTURE V1.87','# RUPTURE V1.88',1)
s=s.replace('**V1.87 visual update:** the BEND page now uses a more realistic Mount-Everest-style eruption sprite sequence derived from the approved PNG sheet. This is a UI-only sprite swap: the K3 curve presets from V1.86.1 remain intact, the OTA connection fix is still retained, and the audio engine / DSP path remain untouched.',
'''**V1.88 BEND visuals:** K3 now controls five distinct 8-frame monochrome animations: LINEAR Everest, SOFT Monolith, HARD pirate flagship erupting from a meadow, S-CURVE close-up camera rising from hull to pirate flag, EXP Kraken. Four eruptions shake; the ship close-up stays stable. All frames are packed read-only 1-bit ROM bitmaps with no additional audio/DSP/ISR work. The V1.86 connection/OTA fix and dual JEDEC ID support are retained.''',1)
p.write_text(s)
p=R/'BEND-PATCH-NOTES.md';s=p.read_text()
s+='''
## V1.88: 5 curve-specific animations

Each K3 curve chooses its own eight-frame sprite set: LINEAR Everest (unchanged V1.87), SOFT monolith, HARD pirate flagship erupting from a meadow, S-CURVE ship close-up camera from hull to flag (no shake), EXP Kraken. Offline-generated 1-bit 224×30 frames in `vendor/etch-v1.88-source/firmware/fm1-native/bend_sprites_v188/` and packed read-only as a 5×8×30×28 uint8 array in `rupture_bend_sprites.h`. A four-phase native UI panel offset is applied when bending except S-CURVE; the real-time audio/FX and ISR code is untouched. The screen still respects existing audio-first LCD refresh throttling. Physical FM1 validation is required.
'''
p.write_text(s)
p=R/'CHEAT-SHEET.html';s=p.read_text();needle='<section><h2>V1.87: EVEREST BEND</h2>'
assert needle in s
s=s.replace(needle,'''<section><h2>V1.88: BEND ANIMATION SET</h2><p>K3 LINEAR: Everest. SOFT: monolith eruption. HARD: old pirate flagship emerges from meadow. S-CURVE: steady close-up camera rises from the ship's ornate cannon deck to its Jolly Roger. EXP: Kraken erupting from the earth. Five unique 8-frame original 224x30 1-bit ROM sprite sequences. All except S-CURVE shake. No new audio DSP/ISR load. OTA connection fix and two FM1 flashchip IDs retained.</p></section>\n<section><h2>V1.87: EVEREST BEND</h2>''',1)
p.write_text(s)
p=R/'WEBSITE-DEPLOY.md';s=p.read_text().replace('RUPTURE-V1.87.fwsc','RUPTURE-V1.88.fwsc').replace('RUPTURE-V1.87-WEBSITE-GITHUB-PAGES.zip','RUPTURE-V1.88-GITHUB-PAGES.zip')
p.write_text(s)
print('PASS V1.88 docs and release notes prepared')
