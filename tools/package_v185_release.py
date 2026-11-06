#!/usr/bin/env python3
"""Assemble V1.85 from existing trusted V1.84.2 OTA assets.
No changes to protected bootheader, loader, project flash map or PC6 path.
"""
from pathlib import Path
import sys,hashlib,json,base64,re
root=Path(__file__).resolve().parents[1]
native=root/'vendor/etch-v1.88-source/firmware/fm1-native'
build=root/'vendor/etch-v1.88-source/build/fm1-native-storage-updater-uac48-p05/app.bin'
app=build.read_bytes()
for name,old_hash in json.loads((root/'reference/V1.84.2-preserved-native-sha256.json').read_text()).items():
 assert hashlib.sha256((native/name).read_bytes()).hexdigest()==old_hash,f'Preserved source changed: {name}'
sys.path.insert(0,str(root/'vendor/etch-v1.88-source/tools'))
import fm1_fwsc as fw
ref=root/'RUPTURE-V1.84.2.fwsc'
assert ref.is_file()
assert not fw.verify_fwsc(ref)[1]
assets=fw.extract_stock_assets(ref)
assert assets['product']=='FM-1_968'
original={e['name']:e['data'] for e in fw.parse_ufw(ref.read_bytes())['entries']}
flash=fw.build_flash_image_from_assets(app,assets['uboot'],assets['cfg_tool'],assets['eq_cfg'],assets['key'],isd_config=assets['isd_config'],flash_header_payload=assets['flash_header_payload'])
assert flash[:0x4000]==original['flash.bin'][:0x4000]
assert len(flash)==len(original['flash.bin'])==0x93000
pkg=fw.build_fwsc(flash,assets['ota'],'FM-1_968')
new=root/'RUPTURE-V1.85.fwsc';new.write_bytes(pkg)
assert not fw.verify_fwsc(new)[1]
entries={e['name']:e['data'] for e in fw.parse_ufw(pkg)['entries']}
assert entries['ota.bin']==original['ota.bin']
assert entries['flash.bin'][:0x4000]==original['flash.bin'][:0x4000]
assert entries['flash.bin']==flash
sha=lambda b:hashlib.sha256(b).hexdigest()
manpath=root/'RELEASE-MANIFEST.json';manifest=json.loads(manpath.read_text())
manifest.update(version='1.85',firmware_file=new.name,firmware_bytes=len(pkg),firmware_sha256=sha(pkg),app_bytes=len(app),app_sha256=sha(app),physical_fm1_verified=False,
 release_note='V1.85: 32-event USB MIDI burst budget, exact outbound panel MIDI no-change fast path with retry, optional scope de-prioritization, read-only detail DMA diagnostics. Three reverbs, analog speaker-pop mitigation, Welcome, project storage and OTA preserved.')
warning='V1.85: only exceptional >32-event MIDI burst input can be deferred by 1+ 128-sample audio halves; queue overflow panic protection remains. Host test does not prove zero DMA LATE/GUARD, no speaker pop or worst-case USB+TRS MIDI stability on real FM1 hardware.'
if warning not in manifest.setdefault('limitations',[]):manifest['limitations'].append(warning)
manpath.write_text(json.dumps(manifest,ensure_ascii=False,indent=2)+'\n')
installer=root/'WebInstaller.html';s=installer.read_text()
assert 'V1.84.2' in s or 'V1.85' in s
s=s.replace('V1.84.2','V1.85')
s,n=re.subn(r'(<script id="firmware"[^>]*>).*?(</script>)',lambda m:m.group(1)+base64.b64encode(pkg).decode('ascii')+m.group(2),s,count=1,flags=re.S);assert n==1
s,n=re.subn(r"const PKG_HASH='[0-9a-f]{64}'",f"const PKG_HASH='{sha(pkg)}'",s);assert n==1
s,n=re.subn(r'const PKG_LEN=\d+',f'const PKG_LEN={len(pkg)}',s);assert n==1
installer.write_text(s)
assert base64.b64decode(re.search(r'<script id="firmware"[^>]*>(.*?)</script>',s,re.S)[1])==pkg
for name,data in [('HEADER',flash[:0x4000]),('TRUSTED OTA',entries['ota.bin'])]:print(name,len(data),sha(data),'UNCHANGED')
print('V1.85 FWSC',len(pkg),sha(pkg),'APP',len(app),sha(app))
