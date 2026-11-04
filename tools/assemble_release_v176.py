#!/usr/bin/env python3
"""RUPTURE V1.76 packaging: exact V1.71 protected head/loader, new app only.

The original user-uploaded V1.71 .fwsc is verified and used as a known-good
container reference. NEVER alter boot region, OTA loader or storage layout.
"""
from pathlib import Path
import sys, json, hashlib, base64, re, shutil
root=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(root/'vendor/etch-v1.88-source/tools'))
import fm1_fwsc as fw
old=root/'reference/RUPTURE-V1.71.fwsc'
app_path=root/'vendor/etch-v1.88-source/build/fm1-native-storage-updater-uac48-p05/app.bin'
new=root/'RUPTURE-V1.76.fwsc'
assert fw.verify_fwsc(old)[1]==[], 'V1.71 input FWSC failed integrity checks'
assets=fw.extract_stock_assets(old)
assert assets['product']=='FM-1_968'
old_pkg=fw.parse_ufw(old.read_bytes());old_e={e['name']:e['data'] for e in old_pkg['entries']}
new_flash=fw.build_flash_image_from_assets(
 app_path.read_bytes(),assets['uboot'],assets['cfg_tool'],assets['eq_cfg'],
 assets['key'],isd_config=assets['isd_config'],
 flash_header_payload=assets['flash_header_payload'])
assert new_flash[:0x4000]==old_e['flash.bin'][:0x4000], '16 KiB protected bootheader changed'
assert new_flash[0x97000 if len(new_flash)>0x97000 else len(new_flash):]==old_e['flash.bin'][0x97000 if len(new_flash)>0x97000 else len(new_flash):], 'Unexpected storage region'
new_pkg=fw.build_fwsc(new_flash,assets['ota'],'FM-1_968')
new.write_bytes(new_pkg)
report,errs=fw.verify_fwsc(new)
assert not errs, f'FWSC invalid: {errs}'
new_e={e['name']:e['data'] for e in fw.parse_ufw(new_pkg)['entries']}
assert new_e['ota.bin']==old_e['ota.bin'], 'OTA loader changed'
assert new_e['flash.bin'][:0x4000]==old_e['flash.bin'][:0x4000]
assert len(new_e['flash.bin'])==len(old_e['flash.bin'])==0x93000
sha=lambda blob:hashlib.sha256(blob).hexdigest()
manifest=json.loads((root/'RELEASE-MANIFEST.json').read_text())
manifest['version']='1.76'
manifest['firmware_file']=new.name
manifest['firmware_bytes']=new.stat().st_size
manifest['firmware_sha256']=sha(new_pkg)
manifest['app_bytes']=app_path.stat().st_size
manifest['app_sha256']=sha(app_path.read_bytes())
manifest['physical_fm1_verified']=False
manifest['release_note']='V1.76: nearest-grid live note quantization with key timestamps/swing; downbeat note handoff after two-bar count-in; faster half-bright LED multiplex scan; preserved trusted OTA and storage.'
(root/'RELEASE-MANIFEST.json').write_text(json.dumps(manifest,indent=2,ensure_ascii=False)+'\n')
# Replace solely the embedded firmware and its hashes. Maintain original JS
# OTA CHECK/WRITE engine as-is to preserve real cross-firmware behavior.
installer=(root/'reference/RUPTURE-V1.71-WebInstaller.html').read_text(encoding='utf-8')
old_fw=old.read_bytes(); old_b64=base64.b64encode(old_fw).decode()
new_b64=base64.b64encode(new_pkg).decode()
assert installer.count(old_b64)==1,'Original embedded FWSC did not match source release'
installer=installer.replace(old_b64,new_b64)
old_sha=sha(old_fw)
assert old_sha in installer
installer=installer.replace(old_sha,sha(new_pkg))
# Replace length ONLY in literal PKG_LEN, never globally in protocol/source.
needle=f'const PKG_LEN={len(old_fw)}'
assert installer.count(needle)==1
installer=installer.replace(needle,f'const PKG_LEN={len(new_pkg)}')
installer=installer.replace('V1.71','V1.76').replace('v1.71','v1.76')
(root/'WebInstaller.html').write_text(installer)
# byte-for-byte OTA JS engine test. The second embedded script must not change.
script=lambda txt:re.findall(r'<script\\b[^>]*>.*?</script\\s*>',txt,re.S|re.I)
# use source-level regular expression (avoid regex/escape transport ambiguity)
from importlib.util import spec_from_file_location,module_from_spec
spec=spec_from_file_location('site',root/'tools/build_site.py');module=module_from_spec(spec);spec.loader.exec_module(module)
original=module.script_parts((root/'WebInstaller.html').read_text())
assert original[0].count(new_b64)==1
print('V1.76 FWSC: old OTA unmodified, 16KiB boot region identical; new app',app_path.stat().st_size,'bytes')
print('FWSC',new.stat().st_size,'bytes sha256',sha(new_pkg))
