#!/usr/bin/env python3
"""RUPTURE V1.85.2 chip-compatibility release: preserve V1.85.1 loader/header."""
from pathlib import Path
import base64,hashlib,json,re,sys,zipfile
root=Path(__file__).resolve().parents[1]
base=root/'reference/RUPTURE-V1.85.1.fwsc'
if not base.is_file():base=root/'RUPTURE-V1.85.1.fwsc'
assert base.is_file()
app=(root/'vendor/etch-v1.88-source/build/fm1-native-storage-updater-uac48-p05/app.bin').read_bytes()
sys.path.insert(0,str(root/'vendor/etch-v1.88-source/tools'))
import fm1_fwsc as fw
assert fw.verify_fwsc(base)[1]==[]
a=fw.extract_stock_assets(base)
entries_old={e['name']:e['data'] for e in fw.parse_ufw(base.read_bytes())['entries']}
assert a['product']=='FM-1_968'
flash=fw.build_flash_image_from_assets(app,a['uboot'],a['cfg_tool'],a['eq_cfg'],a['key'],isd_config=a['isd_config'],flash_header_payload=a['flash_header_payload'])
assert len(flash)==len(entries_old['flash.bin'])==0x93000
assert flash[:0x4000]==entries_old['flash.bin'][:0x4000]
pack=fw.build_fwsc(flash,a['ota'],'FM-1_968')
new=root/'RUPTURE-V1.85.2.fwsc';new.write_bytes(pack)
assert fw.verify_fwsc(new)[1]==[]
entries_new={e['name']:e['data'] for e in fw.parse_ufw(pack)['entries']}
assert entries_new['ota.bin']==entries_old['ota.bin']
assert entries_new['flash.bin'][:0x4000]==entries_old['flash.bin'][:0x4000]
assert entries_new['flash.bin']==flash
sha=lambda b:hashlib.sha256(b).hexdigest()
mfile=root/'RELEASE-MANIFEST.json';m=json.loads(mfile.read_text())
m.update(version='1.85.2',firmware_file=new.name,firmware_bytes=len(pack),firmware_sha256=sha(pack),app_bytes=len(app),app_sha256=sha(app),physical_fm1_verified=False,
 release_note='V1.85.2: recognize both Puya 0x856014 and UCUN UC25HQ80 0xB36014; read/cached ID at boot, allow both in confirmed flash-write policy, report JEDEC via OTA identity bytes 24..26. Preserve existing stock-compatible prebuilt OTA loader and firmware audio, UI and project layout.')
warning='V1.85.2: UC25HQ80 acceptance is source/host-tested only; save/reboot/OTA on a physical 0xB36014 FM1 requires validation. Existing trusted OTA loader has no independent JEDEC check, and was left unchanged. An unreadable JEDEC probe (0) keeps the previously working write behavior for backwards compatibility.'
if warning not in m.setdefault('limitations',[]):m['limitations'].append(warning)
mfile.write_text(json.dumps(m,indent=2,ensure_ascii=False)+'\n')
installer=root/'WebInstaller.html';s=installer.read_text()
assert 'V1.85.1' in s
s=s.replace('V1.85.1','V1.85.2')
s,n=re.subn(r'(<script id="firmware"[^>]*>).*?(</script>)',lambda x:x.group(1)+base64.b64encode(pack).decode('ascii')+x.group(2),s,count=1,flags=re.S);assert n==1
s,n=re.subn(r"const PKG_HASH='[0-9a-f]{64}'",f"const PKG_HASH='{sha(pack)}'",s);assert n==1
s,n=re.subn(r'const PKG_LEN=\d+',f'const PKG_LEN={len(pack)}',s);assert n==1
installer.write_text(s)
assert base64.b64decode(re.search(r'<script id="firmware"[^>]*>(.*?)</script>',s,re.S).group(1))==pack
print('OLD FWSC',sha(base.read_bytes()), 'NEW FWSC',sha(pack))
print('APP',len(app),sha(app),'FWSC',len(pack),sha(pack))
print('BOOT HEADER',len(flash[:0x4000]),sha(flash[:0x4000]),'UNCHANGED')
print('OTA loader',len(entries_new['ota.bin']),sha(entries_new['ota.bin']),'UNCHANGED')
