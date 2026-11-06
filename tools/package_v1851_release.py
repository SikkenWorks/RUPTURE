#!/usr/bin/env python3
"""V1.85.1 LENGTH=4 audio-only hotfix; reuse V1.85 boot/OTA bytes.

Never rewrites application code besides the separately audited audio_core.c.
"""
from pathlib import Path
import base64,hashlib,json,re,sys
ROOT=Path(__file__).resolve().parents[1]
APP=ROOT/'vendor/etch-v1.88-source/build/fm1-native-storage-updater-uac48-p05/app.bin'
OLD=ROOT/'reference/RUPTURE-V1.85.fwsc'
NEW=ROOT/'RUPTURE-V1.85.1.fwsc'
sha=lambda b:hashlib.sha256(b).hexdigest()
assert APP.is_file() and OLD.is_file()
hashes=json.loads((ROOT/'reference/V1.85-firmware-source-sha256.json').read_text())
differences=[p for p,old_hash in hashes.items() if sha((ROOT/p).read_bytes())!=old_hash]
assert differences==['vendor/etch-v1.88-source/firmware/fm1-native/audio_core.c'],differences
sys.path.insert(0,str(ROOT/'vendor/etch-v1.88-source/tools'))
import fm1_fwsc as fw
assert not fw.verify_fwsc(OLD)[1]
a=fw.extract_stock_assets(OLD)
assert a['product']=='FM-1_968'
entries_old={e['name']:e['data'] for e in fw.parse_ufw(OLD.read_bytes())['entries']}
app=APP.read_bytes()
flash=fw.build_flash_image_from_assets(app,a['uboot'],a['cfg_tool'],a['eq_cfg'],a['key'],isd_config=a['isd_config'],flash_header_payload=a['flash_header_payload'])
assert len(flash)==len(entries_old['flash.bin'])==0x93000
assert flash[:0x4000]==entries_old['flash.bin'][:0x4000]
pack=fw.build_fwsc(flash,a['ota'],'FM-1_968')
NEW.write_bytes(pack)
assert not fw.verify_fwsc(NEW)[1]
entries_new={e['name']:e['data'] for e in fw.parse_ufw(pack)['entries']}
assert entries_new['ota.bin']==entries_old['ota.bin']
assert entries_new['flash.bin'][:0x4000]==entries_old['flash.bin'][:0x4000]
assert entries_new['flash.bin']==flash
manifest_path=ROOT/'RELEASE-MANIFEST.json'
m=json.loads(manifest_path.read_text())
m.update(version='1.85.1', firmware_file=NEW.name,firmware_bytes=len(pack),firmware_sha256=sha(pack),
         app_bytes=len(app),app_sha256=sha(app),physical_fm1_verified=False,
         release_note='V1.85.1: fixes sequencer LENGTH=4 playing 16 steps; clamp audio playhead after shortening LENGTH while playing. No other firmware sources, bootloader, OTA, UI, sound, effects or storage changes.')
warning='V1.85.1: sequencer length regression is confirmed and fixed by native audio host tests; physical FM1 playback validation remains required.'
if warning not in m.setdefault('limitations',[]):m['limitations'].append(warning)
manifest_path.write_text(json.dumps(m,indent=2,ensure_ascii=False)+'\n')
web=ROOT/'WebInstaller.html'
s=(ROOT/'reference/WebInstaller-V1.85.html').read_text()
assert 'V1.85' in s
s=s.replace('V1.85','V1.85.1')
s,n=re.subn(r'(<script id="firmware"[^>]*>).*?(</script>)',lambda match:match.group(1)+base64.b64encode(pack).decode('ascii')+match.group(2),s,count=1,flags=re.S)
assert n==1
s,n=re.subn(r"const PKG_HASH='[0-9a-f]{64}'",f"const PKG_HASH='{sha(pack)}'",s);assert n==1
s,n=re.subn(r'const PKG_LEN=\d+',f'const PKG_LEN={len(pack)}',s);assert n==1
web.write_text(s)
assert base64.b64decode(re.search(r'<script id="firmware"[^>]*>(.*?)</script>',s,re.S).group(1))==pack
print('V1.85.1',len(pack),'FWSC SHA256',sha(pack),'APP',len(app),'APP SHA256',sha(app))
print('Header and trusted loader unchanged; native code changes: audio_core.c only')
