#!/usr/bin/env python3
"""V1.42 native app only, exact protected header and OTA preservation."""
from pathlib import Path
import hashlib,json,sys,base64,re,shutil
r=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(r/'vendor/etch-v1.88-source/tools'))
import fm1_fwsc as fw
ref=r/'reference/RUPTURE-V1.41.fwsc'
appdir=r/'vendor/etch-v1.88-source/build/fm1-native-storage-updater-uac48-p05'
app=(appdir/'app.bin').read_bytes()
report,errs=fw.verify_fwsc(ref)
assert not errs and report['product']=='FM-1_968',errs
assets=fw.extract_stock_assets(ref)
old={e['name']:e['data'] for e in fw.parse_ufw(ref.read_bytes())['entries']}
flash=fw.build_flash_image_from_assets(app,assets['uboot'],assets['cfg_tool'],assets['eq_cfg'],assets['key'],isd_config=assets['isd_config'],flash_header_payload=assets['flash_header_payload'])
assert flash[:0x4000]==old['flash.bin'][:0x4000]
pkg=fw.build_fwsc(flash,old['ota.bin'],'FM-1_968')
out=r/'RUPTURE-V1.42.fwsc';out.write_bytes(pkg)
verified,errs=fw.verify_fwsc(out)
assert not errs and verified['flash']['app_crc_ok'] and verified['product']=='FM-1_968',errs
new={e['name']:e['data'] for e in fw.parse_ufw(pkg)['entries']}
assert new['flash.bin'][:0x4000]==old['flash.bin'][:0x4000]
assert new['ota.bin']==old['ota.bin']
sha=hashlib.sha256(pkg).hexdigest()
manifest=json.loads((r/'RELEASE-MANIFEST.json').read_text())
manifest.update(version='1.42',firmware_file=out.name,firmware_bytes=len(pkg),firmware_sha256=sha,app_bytes=len(app),app_sha256=hashlib.sha256(app).hexdigest(),physical_fm1_verified=False,hardware_verified=False)
manifest['limitations'].append('V1.42 smooths OCTAVE FUZZ gain, drive, octave and tone changes at sample rate, softens the rectifier zero crossing, and replaces its periodic variable division with a fixed reciprocal lookup. The original clipping character is retained. Real FM1 listening and DMA timing must still be verified.')
manifest['octave_fuzz_v142']={'makeup':'sample-rate smoothed target, no periodic gain step', 'rectifier':'C1 smooth near zero crossing', 'controls':'per-sample ramp from UI-precomputed targets','division_in_audio_ratio':False,'physical_fm1_verified':False}
(r/'RELEASE-MANIFEST.json').write_text(json.dumps(manifest,indent=2)+'\n')
html=(r/'WebInstaller.html').read_text()
html,n=re.subn(r'(<script id="firmware" type="application/octet-stream">).*?(</script>)',lambda m:m.group(1)+base64.b64encode(pkg).decode()+m.group(2),html,count=1,flags=re.S);assert n==1
html=html.replace('V1.41','V1.42')
html,n=re.subn(r"(const PKG_HASH=')[0-9a-f]{64}(';const PKG_LEN=)\d+(;)",lambda m:m.group(1)+sha+m.group(2)+str(len(pkg))+m.group(3),html,count=1);assert n==1
assert 'experimental rupture' not in html.lower()
(r/'WebInstaller.html').write_text(html)
for name in ['app.bin','app.bin.audit.json','native-build-options.json']:
 p=r/'native-build'/name;p.parent.mkdir(exist_ok=True);shutil.copy2(appdir/name,p)
print('PASS V1.42 FWSC',len(pkg),'bytes SHA-256',sha,'protected header/OTA unchanged')
