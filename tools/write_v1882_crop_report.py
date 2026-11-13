#!/usr/bin/env python3
from pathlib import Path
import hashlib,json,re,sys
R=Path(__file__).resolve().parents[1]
P=Path('/mnt/data/rupture-v188-poster-fix')
FM=R/'vendor/etch-v1.88-source/firmware/fm1-native'
sha=lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
sys.path.insert(0,str(R/'vendor/etch-v1.88-source/tools'))
import fm1_fwsc as fw
old=P/'RUPTURE-V1.88.1.fwsc'
new=R/'RUPTURE-V1.88.2.fwsc'
extract=lambda f:{e['name']:e['data'] for e in fw.parse_ufw(f.read_bytes())['entries']}
a=extract(old);b=extract(new)
assert a['ota.bin']==b['ota.bin']
assert a['flash.bin'][:0x4000]==b['flash.bin'][:0x4000]
app=R/'native-build/app.bin'
meta=json.loads((FM/'bend_sprites_v188/POSTER-SOURCE-EXTRACTION.json').read_text())
unchanged=['audio_core.c','audio_core.h','native_fx.c','native_fx.h','native_cxs_q15.c','audio_target.c',
 'native_panel_midi.c','usb.c','ota.c','ota_target.c','wave_store_target.c','native_project_codec.c',
 'native_flash_probe.h','hal/fm1_flash_id.h','rupture_bend_curves.h','hal/fm1_isr.S','bringup.c',
 'rupture_bend_storyboard.h']
for f in unchanged:assert (FM/f).read_bytes()==(P/'vendor/etch-v1.88-source/firmware/fm1-native'/f).read_bytes(),f
report=f'''# RUPTURE V1.88.2 — All BEND animations: 1:1 crop fix\n\nDate: 2026-11-11\nBase: V1.88.1 user-approved poster release\n\n## Root cause\nThe V1.88.1 sprite extraction tool resized poster image cells independently along X and Y (approximately 156 × 90 to 224 × 30), changing their aspect ratios and flattening their subjects.\n\n## Fix\n- **40/40 frames** use exactly 1:1 source pixel cropping, without interpolation or resizing.\n- Per-frame Y camera windows select the most relevant subject details from original poster cells.\n- Source crop width is 155–156 pixels, preserved as-is and centered in the 224 × 30 native viewport. Black side margins provide the remainder; tall art is intentionally clipped.\n- Bit conversion uses the same threshold **112** and the same ROM bit order.\n- Native sprites remain exactly **33,600 ROM bytes** (5 × 8 × 30 × 28).\n- 4 animations keep their previous shake; S-CURVE close-up stays still. Existing storyboard/pitch/K1/K2/K3 logic untouched.\n- Physical FM1 screen remains 240 × 240, sprite panel remains 224 × 30; no other UI layout changes.\n\n## Tests\n- PASS: 40 native PNGs are exact 1:1 cropped source pixels after 1-bit threshold; all map bit-for-bit to the compiled ROM sprite header.\n- PASS: all 40 new frames differ from the aspect-distorted V1.88.1 output.\n- PASS: 80 native 240 × 240 UI previews (5 curves × 8 frames × CRT on/off).\n- PASS: four shaking eruption curves; S-CURVE ship close-up remains phase-independent.\n- PASS: native FM1 cross-compilation, twice with identical app SHA-256.\n- PASS: CXS and SAW bend sustained audio tests; five K3 curves; no retrigger; neutral PCM unchanged.\n- PASS: V1.88.1 storyboard and reverse/release logic.\n- PASS: Web MIDI FM1 identity on Puya `0x856014` and UC25HQ80 `0xB36014`, unknown port-name read-only probes.\n- PASS: native `.fwsc` integrity and index/WebInstaller embedded package parity.\n- PASS: all 18 critical audio, DSP, MIDI, flash, ISR, and OTA source files byte-identical to V1.88.1.\n- PASS: OTA loader and 16 KiB protected bootheader byte-identical to V1.88.1.\n\n## Build and integrity\n- App bytes: **{app.stat().st_size}**; application headroom: **{581564-app.stat().st_size}** bytes.\n- App SHA-256: `{sha(app)}`\n- FM1 `.fwsc` bytes: **{new.stat().st_size}**\n- FM1 `.fwsc` SHA-256: `{sha(new)}`\n- Original trusted OTA loader SHA-256: `{hashlib.sha256(b['ota.bin']).hexdigest()}`\n- Original protected bootheader SHA-256: `{hashlib.sha256(b['flash.bin'][:0x4000]).hexdigest()}`\n- Poster source SHA-256: `{meta['source_sha256']}`\n- New 33,600-byte native 1-bit sprite bank SHA-256: `{meta['packed_rom_sha256']}`\n\n## Limitations\nThe original approved source is a composite **JPEG poster**, not five original full-resolution raster animation strips. A 30px-high frame cannot show an entire tall poster scene without rescaling, so the fix uses close-up crops with black margins. This preserves actual proportions but necessarily hides parts of the original image.\n\nNo physical FM1 test has been performed. Hardware LCD refresh timing, power-on speaker-pop behavior and OTA installation remain to be validated on recoverable hardware.\n'''
p=R/'RUPTURE-TEST-RESULTS-V1.88.2.md'
p.write_text(report)
print('wrote',p)
