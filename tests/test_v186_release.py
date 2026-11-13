#!/usr/bin/env python3
"""V1.86 post-build OTA safety and release integrity; no physical FM1 claims."""
from pathlib import Path
import base64, hashlib, json, re, sys
r=Path(__file__).resolve().parents[1]
v='1.86'
sys.path.insert(0,str(r/'vendor/etch-v1.88-source/tools'))
import fm1_fwsc as fw
sha=lambda b:hashlib.sha256(b).hexdigest()
new=(r/f'RUPTURE-V{v}.fwsc').read_bytes()
old=(r/'reference/RUPTURE-V1.85.2.fwsc').read_bytes()
meta=json.loads((r/'RELEASE-MANIFEST.json').read_text())
app=(r/'vendor/etch-v1.88-source/build/fm1-native-storage-updater-uac48-p05/app.bin').read_bytes()
assert meta['version']==v and meta['firmware_file']==f'RUPTURE-V{v}.fwsc'
assert meta['firmware_sha256']==sha(new) and meta['firmware_bytes']==len(new)
assert meta['app_sha256']==sha(app) and meta['app_bytes']==len(app)
assert (r/'native-build/app.bin').read_bytes()==app
assert not meta['physical_fm1_verified'] and not meta['release_ready']
assert fw.verify_fwsc(r/f'RUPTURE-V{v}.fwsc')[1]==[]
assert fw.verify_fwsc(r/'reference/RUPTURE-V1.85.2.fwsc')[1]==[]
entries=lambda pack:{x['name']:x['data'] for x in fw.parse_ufw(pack)['entries']}
e,e0=entries(new),entries(old)
assert e['ota.bin']==e0['ota.bin']
assert e['flash.bin'][:0x4000]==e0['flash.bin'][:0x4000]
assert len(e['flash.bin'])==0x93000
assets=fw.extract_stock_assets(r/'reference/RUPTURE-V1.85.2.fwsc')
expected=fw.build_flash_image_from_assets(app,assets['uboot'],assets['cfg_tool'],assets['eq_cfg'],assets['key'],
    isd_config=assets['isd_config'],flash_header_payload=assets['flash_header_payload'])
assert e['flash.bin']==expected
installer=(r/'WebInstaller.html').read_text()
site=(r/'index.html').read_text()
for label,data in [('WebInstaller',installer),('website',site)]:
 embedded=base64.b64decode(re.search(r'<script id="firmware"[^>]*>(.*?)</script>',data,re.S).group(1))
 assert embedded==new, label
 assert f"const PKG_HASH='{sha(new)}'" in data
 assert f'const PKG_LEN={len(new)}' in data
assert 'V1.86' in installer
src=r/'vendor/etch-v1.88-source/firmware/fm1-native'
assert '"V1.86"' in (src/'rupture_ui.c').read_text()
assert 'RU_PAGE_BEND' in (src/'rupture_ui.c').read_text()
assert 'live_bend_q16' in (src/'audio_core.c').read_text()
assert (src/'rupture_bend_sprites.h').exists()
assert (src/'hal/fm1_flash_id.h').exists()
assert list((src/'bend_frames').glob('bend_frame_*.png'))
print('PASS V1.86 app 357k + FWSC checksum, firmware metadata, WebInstaller/site parity')
print('PASS V1.86 trusted OTA payload + protected header byte-identical to V1.85.2')
print('PASS V1.86 BEND pre-FX source, sprites, and inherited dual flash ID helper')
