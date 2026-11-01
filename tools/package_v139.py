#!/usr/bin/env python3
"""Package V1.39 app only; do not modify V1.38 protected header/OTA loader."""
from pathlib import Path
import hashlib,json,sys,base64,re
r=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(r/'vendor/etch-v1.88-source/tools'))
import fm1_fwsc as fw
ref=r/'RUPTURE-V1.38.fwsc'
app=(r/'vendor/etch-v1.88-source/build/fm1-native-storage-updater-uac48-p05/app.bin').read_bytes()
report,errs=fw.verify_fwsc(ref)
assert not errs and report['product']=='FM-1_968', errs
assets=fw.extract_stock_assets(ref)
old={e['name']:e['data'] for e in fw.parse_ufw(ref.read_bytes())['entries']}
flash=fw.build_flash_image_from_assets(app,assets['uboot'],assets['cfg_tool'],assets['eq_cfg'],assets['key'],isd_config=assets['isd_config'],flash_header_payload=assets['flash_header_payload'])
assert flash[:0x4000]==old['flash.bin'][:0x4000], 'protected boot sector changed'
pkg=fw.build_fwsc(flash,old['ota.bin'],'FM-1_968')
out=r/'RUPTURE-V1.39.fwsc';out.write_bytes(pkg)
verified,errs=fw.verify_fwsc(out)
assert not errs and verified['flash']['app_crc_ok'] and verified['product']=='FM-1_968',errs
new={e['name']:e['data'] for e in fw.parse_ufw(pkg)['entries']}
assert new['flash.bin'][:0x4000]==old['flash.bin'][:0x4000]
assert new['ota.bin']==old['ota.bin']
sha=hashlib.sha256(pkg).hexdigest()
manifest=json.loads((r/'RELEASE-MANIFEST.json').read_text())
manifest.update(version='1.39',firmware_file=out.name,firmware_bytes=len(pkg),firmware_sha256=sha,
                app_bytes=len(app),app_sha256=hashlib.sha256(app).hexdigest(),physical_fm1_verified=False,hardware_verified=False)
manifest['limitations'].append('V1.39 fixes a confirmed DSP hot-path defect: enabled but flat Channel EQ 2.0 previously disabled full FX dry bypass and processed four Q26 biquads; new dry unity and per-band neutral paths are proven in host tests, but FM1 DMA timing/physical crackle are not yet verified.')
manifest['limitations'].append('V1.39 targets the no-filter/no-effects audio regression; intermittent hardware DMA stops require reading AUDIO DIAGNOSTICS on the actual FM1 to prove resolution.')
(r/'RELEASE-MANIFEST.json').write_text(json.dumps(manifest,indent=2)+'\n')
html=(r/'WebInstaller.html').read_text()
html,n=re.subn(r'(<script id="firmware" type="application/octet-stream">).*?(</script>)',lambda m:m.group(1)+base64.b64encode(pkg).decode()+m.group(2),html,count=1,flags=re.S)
assert n==1
html=html.replace('V1.38','V1.39')
html,n=re.subn(r"(const PKG_HASH=')[0-9a-f]{64}(';const PKG_LEN=)\d+(;)",lambda m:m.group(1)+sha+m.group(2)+str(len(pkg))+m.group(3),html,count=1)
assert n==1
assert 'experimental RUPTURE' not in html
(r/'WebInstaller.html').write_text(html)
print('PASS fwsc',len(pkg),'sha256',sha,'protected header exact, OTA loader exact')
