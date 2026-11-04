#!/usr/bin/env python3
"""RUPTURE V1.82 reproducible native-app FWSC packaging.
The V1.80 known working protected head and OTA loader are immutable here.
"""
from pathlib import Path
import sys,hashlib,json,base64,re,shutil
root=Path(__file__).resolve().parents[1]
app=(root/'vendor/etch-v1.88-source/build/fm1-native-storage-updater-uac48-p05/app.bin').read_bytes()
old=root/'RUPTURE-V1.81.fwsc'
sys.path.insert(0,str(root/'vendor/etch-v1.88-source/tools'))
import fm1_fwsc as fw
assert not fw.verify_fwsc(old)[1]
assets=fw.extract_stock_assets(old)
assert assets['product']=='FM-1_968'
old_e={e['name']:e['data'] for e in fw.parse_ufw(old.read_bytes())['entries']}
flash=fw.build_flash_image_from_assets(app,assets['uboot'],assets['cfg_tool'],assets['eq_cfg'],assets['key'],isd_config=assets['isd_config'],flash_header_payload=assets['flash_header_payload'])
assert flash[:0x4000]==old_e['flash.bin'][:0x4000], 'protected boot header changed'
assert len(flash)==len(old_e['flash.bin'])==0x93000, 'flash layout changed'
assert flash[:0x4000]==old_e['flash.bin'][:0x4000]
package=fw.build_fwsc(flash,assets['ota'],'FM-1_968')
new=root/'RUPTURE-V1.82.fwsc';new.write_bytes(package)
assert not fw.verify_fwsc(new)[1]
new_e={e['name']:e['data'] for e in fw.parse_ufw(package)['entries']}
assert new_e['ota.bin']==old_e['ota.bin']
assert new_e['flash.bin'][:0x4000]==old_e['flash.bin'][:0x4000]
assert flash==new_e['flash.bin']
sha=lambda data:hashlib.sha256(data).hexdigest()
m_path=root/'RELEASE-MANIFEST.json';m=json.loads(m_path.read_text())
m.update(version='1.82',firmware_file=new.name,firmware_bytes=len(package),firmware_sha256=sha(package),app_bytes=len(app),app_sha256=sha(app),physical_fm1_verified=False,
 release_note='V1.82: Welcome first-boot per-version status and non-destructive migration. DSP/OTA/FX unchanged.')
m.setdefault('limitations',[]).append('V1.82 and stock V15 round-trip to the exact same V1.82 cannot always be detected if stock preserves the setup marker; FM1 hardware verification required.')
m_path.write_text(json.dumps(m,indent=2,ensure_ascii=False)+'\n')
p=root/'WebInstaller.html';s=p.read_text()
s,n=re.subn(r'(<script id="firmware"[^>]*>).*?(</script>)',lambda x:x[1]+base64.b64encode(package).decode()+x[2],s,count=1,flags=re.S);assert n==1
s,n=re.subn(r"const PKG_HASH='[0-9a-f]{64}'",f"const PKG_HASH='{sha(package)}'",s);assert n==1
s,n=re.subn(r'const PKG_LEN=\d+',f'const PKG_LEN={len(package)}',s);assert n==1
assert 'V1.82' in s
p.write_text(s)
for filename,blob in [('PROTECTED-BOOT-HEAD',old_e['flash.bin'][:0x4000]),('TRUSTED-OTA-LOADER',old_e['ota.bin'])]:print(filename,len(blob),'SHA256',sha(blob))
print('V1.82 fwsc',len(package),sha(package),'app',len(app),sha(app))
