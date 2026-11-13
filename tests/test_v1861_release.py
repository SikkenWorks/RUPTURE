#!/usr/bin/env python3
"""RUPTURE V1.86.1 release + preserved V1.86 connection fix/OTA audit."""
from pathlib import Path
import hashlib,base64,json,re,sys
r=Path(__file__).resolve().parents[1]
src=r/'vendor/etch-v1.88-source/firmware/fm1-native'
sys.path.insert(0,str(r/'vendor/etch-v1.88-source/tools'))
import fm1_fwsc as fw
sha=lambda b:hashlib.sha256(b).hexdigest()
v='1.86.1';m=json.loads((r/'RELEASE-MANIFEST.json').read_text())
assert m['version']==v and m['firmware_file']==f'RUPTURE-V{v}.fwsc'
new=(r/m['firmware_file']).read_bytes();old=(r/'RUPTURE-V1.86.fwsc').read_bytes()
assert m['firmware_sha256']==sha(new) and m['firmware_bytes']==len(new)
app=(r/'native-build/app.bin').read_bytes()
assert m['app_sha256']==sha(app) and m['app_bytes']==len(app)
assert (r/'vendor/etch-v1.88-source/build/fm1-native-storage-updater-uac48-p05/app.bin').read_bytes()==app
assert fw.verify_fwsc(r/m['firmware_file'])[1]==[]
assert fw.verify_fwsc(r/'RUPTURE-V1.86.fwsc')[1]==[]
ents=lambda b:{e['name']:e['data'] for e in fw.parse_ufw(b)['entries']}
e,e0=ents(new),ents(old)
assert e['ota.bin']==e0['ota.bin']
assert e['flash.bin'][:0x4000]==e0['flash.bin'][:0x4000]
assert len(e['flash.bin'])==len(e0['flash.bin'])==0x93000
assert (src/'rupture_bend_sprites.h').read_bytes()==(Path('/mnt/data/rupture-v186-connection-work/vendor/etch-v1.88-source/firmware/fm1-native/rupture_bend_sprites.h')).read_bytes()
assert (src/'audio_core.c').read_bytes()==(Path('/mnt/data/rupture-v186-connection-work/vendor/etch-v1.88-source/firmware/fm1-native/audio_core.c')).read_bytes()
assert (src/'ota.c').read_bytes()==(Path('/mnt/data/rupture-v186-connection-work/vendor/etch-v1.88-source/firmware/fm1-native/ota.c')).read_bytes()
assert (src/'native_flash_probe.h').read_bytes()==(Path('/mnt/data/rupture-v186-connection-work/vendor/etch-v1.88-source/firmware/fm1-native/native_flash_probe.h')).read_bytes()
for name in ('WebInstaller.html','index.html'):
 html=(r/name).read_text()
 assert base64.b64decode(re.search(r'<script id="firmware"[^>]*>(.*?)</script>',html,re.S).group(1))==new
 assert f"const PKG_HASH='{sha(new)}'" in html and f'const PKG_LEN={len(new)}' in html
 assert 'Stop at the FIRST NUL' in html
 assert 'read-only identity queries' in html
 assert 'V1.86.1' in html
assert 'RU_PAGE_BEND' in (src/'rupture_ui.c').read_text()
assert 'ru_bend_curve_percent(ru_bend_curve,ru_bend_live)' in (src/'bringup.c').read_text()
assert 'ru_bend_curve_names[curve]' in (src/'rupture_ui.c').read_text()
assert not m.get('physical_fm1_verified') and not m.get('release_ready')
print('PASS V1.86.1 release: flash & OTA loader preserved; WebInstaller/Index connected + exact new firmware; original audio_core and mountain sprites retained')
