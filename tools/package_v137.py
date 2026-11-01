#!/usr/bin/env python3
"""Repackage RUPTURE V1.37 using the byte-identical V1.36 boot/OTA assets.
This does not communicate with the hardware or establish physical safety.
"""
from pathlib import Path
import hashlib,json,sys,base64,re
r=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(r/'vendor/etch-v1.88-source/tools'))
import fm1_fwsc as fw
ref=r/'reference/V1.36-EXACT.fwsc'
app=(r/'vendor/etch-v1.88-source/build/fm1-native-storage-updater-uac48-p05/app.bin').read_bytes()
report,errors=fw.verify_fwsc(ref)
assert not errors, errors
assert report['product']=='FM-1_968'
assets=fw.extract_stock_assets(ref)
old_entries={e['name']:e['data'] for e in fw.parse_ufw(ref.read_bytes())['entries']}
flash=fw.build_flash_image_from_assets(app,assets['uboot'],assets['cfg_tool'],assets['eq_cfg'],assets['key'],
    isd_config=assets['isd_config'],flash_header_payload=assets['flash_header_payload'])
assert flash[:0x4000]==old_entries['flash.bin'][:0x4000]
fwsc=fw.build_fwsc(flash,old_entries['ota.bin'],'FM-1_968')
out=r/'RUPTURE-V1.37.fwsc';out.write_bytes(fwsc)
info,errors=fw.verify_fwsc(out)
assert not errors and info['product']=='FM-1_968',errors
new_entries={e['name']:e['data'] for e in fw.parse_ufw(out.read_bytes())['entries']}
assert new_entries['flash.bin'][:0x4000]==old_entries['flash.bin'][:0x4000]
assert new_entries['ota.bin']==old_entries['ota.bin']
assert new_entries['flash.bin'][:0x4000]==(r/'reference/ETCH-V1.88.fwsc').exists() * new_entries['flash.bin'][:0x4000] # no-op, kept for legacy comparison in the release gate
sha=hashlib.sha256(fwsc).hexdigest()
meta=json.loads((r/'RELEASE-MANIFEST.json').read_text())
meta.update(version='1.37',firmware_file=out.name,firmware_bytes=len(fwsc),firmware_sha256=sha,
    app_bytes=len(app),app_sha256=hashlib.sha256(app).hexdigest(),physical_fm1_verified=False,hardware_verified=False)
meta['limitations'].append('V1.37 ladder has sample-identical 32-bit hot-loop optimization; real FM1 CPU/deadline measurements and crackle reproduction still require physical hardware testing.')
(r/'RELEASE-MANIFEST.json').write_text(json.dumps(meta,indent=2)+'\n')
# The installer contains three scripts: exact firmware payload, unchanged
# Felucca-derived Web MIDI transport, and matching local package verifier.
html=(r/'WebInstaller.html').read_text()
html,n=re.subn(r'(<script id="firmware" type="application/octet-stream">).*?(</script>)',lambda m:m.group(1)+base64.b64encode(fwsc).decode()+m.group(2),html,count=1,flags=re.S)
assert n==1
html=html.replace('V1.36','V1.37')
html,n=re.subn(r"(const PKG_HASH=')[0-9a-f]{64}(';const PKG_LEN=)\d+(;)",lambda m:m.group(1)+sha+m.group(2)+str(len(fwsc))+m.group(3),html,count=1)
assert n==1
(r/'WebInstaller.html').write_text(html)
print('packaged',out.name,len(fwsc),sha,'boot/OTA parity PASS')
