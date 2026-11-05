#!/usr/bin/env python3
"""Build V1.83 from the audited V1.82 package; preserve OTA and boot bytes."""
from pathlib import Path
import sys,hashlib,json,base64,re,shutil
root=Path(__file__).resolve().parents[1]
app_path=root/'vendor/etch-v1.88-source/build/fm1-native-storage-updater-uac48-p05/app.bin'
app=app_path.read_bytes();old=root/'RUPTURE-V1.82.fwsc'
sys.path.insert(0,str(root/'vendor/etch-v1.88-source/tools'))
import fm1_fwsc as fw
assert not fw.verify_fwsc(old)[1]
assets=fw.extract_stock_assets(old)
assert assets['product']=='FM-1_968'
old_e={e['name']:e['data'] for e in fw.parse_ufw(old.read_bytes())['entries']}
flash=fw.build_flash_image_from_assets(app,assets['uboot'],assets['cfg_tool'],assets['eq_cfg'],assets['key'],isd_config=assets['isd_config'],flash_header_payload=assets['flash_header_payload'])
assert flash[:0x4000]==old_e['flash.bin'][:0x4000]
assert len(flash)==len(old_e['flash.bin'])==0x93000
pkg=fw.build_fwsc(flash,assets['ota'],'FM-1_968')
new=root/'RUPTURE-V1.83.fwsc';new.write_bytes(pkg)
assert not fw.verify_fwsc(new)[1]
new_e={e['name']:e['data'] for e in fw.parse_ufw(pkg)['entries']}
assert new_e['ota.bin']==old_e['ota.bin']
assert new_e['flash.bin'][:0x4000]==old_e['flash.bin'][:0x4000]
assert flash==new_e['flash.bin']
sha=lambda b:hashlib.sha256(b).hexdigest()
m_path=root/'RELEASE-MANIFEST.json';m=json.loads(m_path.read_text())
m.update(version='1.83',firmware_file=new.name,firmware_bytes=len(pkg),firmware_sha256=sha(pkg),app_bytes=len(app),app_sha256=sha(app),physical_fm1_verified=False,
 release_note='V1.83: mono-specific sample-exact mixer headroom and per-block master target optimization; FX, storage, welcome and OTA unchanged.')
m.setdefault('limitations',[]).append('V1.83: host PCM parity does not prove zero LATE/GUARD on FM1. Full maximum-param stress and hardware measurements are still required.')
m_path.write_text(json.dumps(m,indent=2,ensure_ascii=False)+'\n')
p=root/'WebInstaller.html';s=p.read_text()
s,n=re.subn(r'(<script id="firmware"[^>]*>).*?(</script>)',lambda x:x[1]+base64.b64encode(pkg).decode()+x[2],s,count=1,flags=re.S);assert n==1
s,n=re.subn(r"const PKG_HASH='[0-9a-f]{64}'",f"const PKG_HASH='{sha(pkg)}'",s);assert n==1
s,n=re.subn(r'const PKG_LEN=\d+',f'const PKG_LEN={len(pkg)}',s);assert n==1
assert 'V1.83' in s
p.write_text(s)
for name,blob in [('BOOTHEADER',old_e['flash.bin'][:0x4000]),('OTA-LOADER',old_e['ota.bin'])]:print(name,len(blob),sha(blob))
print('FWSC',len(pkg),sha(pkg),'APP',len(app),sha(app))
