#!/usr/bin/env python3
"""RUPTURE V1.38: package only the crossbuilt app, preserving the proven
V1.37 app-only boot sector and the exact OTA loader bytes.  Offline verifier
only: no hardware is contacted or claimed validated.
"""
from pathlib import Path
import hashlib,json,sys,base64,re
r=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(r/'vendor/etch-v1.88-source/tools'))
import fm1_fwsc as fw
ref=r/'RUPTURE-V1.37.fwsc'
app=(r/'vendor/etch-v1.88-source/build/fm1-native-storage-updater-uac48-p05/app.bin').read_bytes()
report,errs=fw.verify_fwsc(ref)
assert not errs and report['product']=='FM-1_968', errs
assets=fw.extract_stock_assets(ref)
old={e['name']:e['data'] for e in fw.parse_ufw(ref.read_bytes())['entries']}
flash=fw.build_flash_image_from_assets(app,assets['uboot'],assets['cfg_tool'],assets['eq_cfg'],assets['key'],isd_config=assets['isd_config'],flash_header_payload=assets['flash_header_payload'])
assert flash[:0x4000]==old['flash.bin'][:0x4000], 'boothead mismatch'
pkg=fw.build_fwsc(flash,old['ota.bin'],'FM-1_968')
out=r/'RUPTURE-V1.38.fwsc';out.write_bytes(pkg)
result,errs=fw.verify_fwsc(out)
assert not errs and result['flash']['app_crc_ok'] and result['product']=='FM-1_968',errs
now={e['name']:e['data'] for e in fw.parse_ufw(pkg)['entries']}
assert now['flash.bin'][:0x4000]==old['flash.bin'][:0x4000]
assert now['ota.bin']==old['ota.bin']
sha=hashlib.sha256(pkg).hexdigest()
manifest=json.loads((r/'RELEASE-MANIFEST.json').read_text())
manifest.update(version='1.38',firmware_file=out.name,firmware_bytes=len(pkg),firmware_sha256=sha,
                app_bytes=len(app),app_sha256=hashlib.sha256(app).hexdigest(),physical_fm1_verified=False,hardware_verified=False)
manifest['limitations']=[x for x in manifest['limitations'] if not x.startswith('V1.37 ladder has')]
manifest['limitations'].append('V1.38 prepared coefficient reuse preserves all four filter kernels sample-exactly; actual FM1 DMA margin and reported crackling/audio stops are not independently verified on physical hardware.')
manifest['limitations'].append('RUPTURE monophonic DMA adaptation prioritizes UI/scope suppression before continued deadlines; persistent DMA misses still trigger the original safety stop.')
(r/'RELEASE-MANIFEST.json').write_text(json.dumps(manifest,indent=2)+'\n')
html=(r/'WebInstaller.html').read_text()
html,n=re.subn(r'(<script id="firmware" type="application/octet-stream">).*?(</script>)',lambda m:m.group(1)+base64.b64encode(pkg).decode()+m.group(2),html,count=1,flags=re.S)
assert n==1
html=html.replace('V1.37','V1.38')
html,n=re.subn(r"(const PKG_HASH=')[0-9a-f]{64}(';const PKG_LEN=)\d+(;)",lambda m:m.group(1)+sha+m.group(2)+str(len(pkg))+m.group(3),html,count=1)
assert n==1
assert 'experimental RUPTURE' not in html
(r/'WebInstaller.html').write_text(html)
print('PASS fwsc',len(pkg),'sha256',sha,'bootheader exact, OTA loader exact')
