#!/usr/bin/env python3
"""RUPTURE V1.84.2: isolated LED/PC6 current-edge coordination, preserving OTA."""
from pathlib import Path
import sys,hashlib,json,base64,re
root=Path(__file__).resolve().parents[1]
app=(root/'vendor/etch-v1.88-source/build/fm1-native-storage-updater-uac48-p05/app.bin').read_bytes()
ref=root/'reference/RUPTURE-V1.84.1.fwsc'
sys.path.insert(0,str(root/'vendor/etch-v1.88-source/tools'))
import fm1_fwsc as fw
assert not fw.verify_fwsc(ref)[1]
assets=fw.extract_stock_assets(ref)
assert assets['product']=='FM-1_968'
old={e['name']:e['data'] for e in fw.parse_ufw(ref.read_bytes())['entries']}
flash=fw.build_flash_image_from_assets(app,assets['uboot'],assets['cfg_tool'],assets['eq_cfg'],assets['key'],isd_config=assets['isd_config'],flash_header_payload=assets['flash_header_payload'])
assert flash[:0x4000]==old['flash.bin'][:0x4000]
assert len(flash)==len(old['flash.bin'])==0x93000
pkg=fw.build_fwsc(flash,assets['ota'],'FM-1_968')
new=root/'RUPTURE-V1.84.2.fwsc';new.write_bytes(pkg)
assert not fw.verify_fwsc(new)[1]
entries={e['name']:e['data'] for e in fw.parse_ufw(pkg)['entries']}
assert entries['ota.bin']==old['ota.bin']
assert entries['flash.bin'][:0x4000]==old['flash.bin'][:0x4000]
sha=lambda b:hashlib.sha256(b).hexdigest()
m_path=root/'RELEASE-MANIFEST.json';m=json.loads(m_path.read_text())
m.update(version='1.84.2',firmware_file=new.name,firmware_bytes=len(pkg),firmware_sha256=sha(pkg),app_bytes=len(app),app_sha256=sha(app),physical_fm1_verified=False,
 release_note='V1.84.2: no all-on LED spike during audiofault/protected states; wait for stable button LEDs before PC6 analog enable. Synth/FX, OTA, storage untouched.')
m.setdefault('limitations',[]).append('V1.84.2: source/host tests prove LED current-edge and PC6 sequencing, not removal of an intrinsic amplifier-bias pop; low-volume physical FM1 testing remains essential.')
m_path.write_text(json.dumps(m,indent=2,ensure_ascii=False)+'\n')
p=root/'WebInstaller.html';s=p.read_text()
s=s.replace('V1.84.1','V1.84.2')
s,n=re.subn(r'(<script id="firmware"[^>]*>).*?(</script>)',lambda x:x[1]+base64.b64encode(pkg).decode()+x[2],s,count=1,flags=re.S);assert n==1
s,n=re.subn(r"const PKG_HASH='[0-9a-f]{64}'",f"const PKG_HASH='{sha(pkg)}'",s);assert n==1
s,n=re.subn(r'const PKG_LEN=\d+',f'const PKG_LEN={len(pkg)}',s);assert n==1
p.write_text(s)
for name,blob in [('BOOTHEADER',old['flash.bin'][:0x4000]),('OTA-LOADER',old['ota.bin'])]:print(name,len(blob),sha(blob),'IDENTICAL')
print('FWSC',len(pkg),sha(pkg),'APP',len(app),sha(app))
