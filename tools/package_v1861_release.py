#!/usr/bin/env python3
"""Build V1.86.1 .fwsc using verified V1.86 OTA assets and retained connection fix."""
from pathlib import Path
import base64,hashlib,json,re,sys,shutil
root=Path(__file__).resolve().parents[1]
base=root/'RUPTURE-V1.86.fwsc'
appdir=root/'vendor/etch-v1.88-source/build/fm1-native-storage-updater-uac48-p05'
app=(appdir/'app.bin').read_bytes()
sha=lambda b:hashlib.sha256(b).hexdigest()
sys.path.insert(0,str(root/'vendor/etch-v1.88-source/tools'))
import fm1_fwsc as fw
assert fw.verify_fwsc(base)[1]==[]
assert not fw.validate_app_binary(app)
assets=fw.extract_stock_assets(base)
assert assets['product']=='FM-1_968'
e0={x['name']:x['data'] for x in fw.parse_ufw(base.read_bytes())['entries']}
flash=fw.build_flash_image_from_assets(app,assets['uboot'],assets['cfg_tool'],assets['eq_cfg'],assets['key'],isd_config=assets['isd_config'],flash_header_payload=assets['flash_header_payload'])
assert len(flash)==len(e0['flash.bin'])==0x93000
assert flash[:0x4000]==e0['flash.bin'][:0x4000]
pack=fw.build_fwsc(flash,assets['ota'],'FM-1_968')
file=root/'RUPTURE-V1.86.1.fwsc';file.write_bytes(pack)
assert fw.verify_fwsc(file)[1]==[]
e1={x['name']:x['data'] for x in fw.parse_ufw(pack)['entries']}
assert e1['ota.bin']==e0['ota.bin']
assert e1['flash.bin'][:0x4000]==e0['flash.bin'][:0x4000]
assert e1['flash.bin']==flash
for item in ('app.bin','app.bin.audit.json','native-build-options.json'):
 shutil.copy2(appdir/item,root/'native-build'/item)
b=json.loads((root/'native-build/native-build-options.json').read_text())
assert b['version']=='RUPTURE V1.86.1'
assert b['app_sha256']==sha(app)
manifest=root/'RELEASE-MANIFEST.json'
m=json.loads(manifest.read_text())
m.update(version='1.86.1',firmware_file=file.name,firmware_bytes=len(pack),firmware_sha256=sha(pack),app_bytes=len(app),app_sha256=sha(app),physical_fm1_verified=False,hardware_verified=False,release_ready=False,release_stage='SOFTWARE_RC_PENDING_HARDWARE_VALIDATION',release_feature='BEND K3: five pre-FX momentary pitch response curves',release_note='V1.86.1: BEND K3 LINEAR, SOFT, HARD, S-CURVE and EXP; live curve plot; K1 and K2 preserved; same 8 mountain sprites; existing V1.86 MIDI/OTA installer connection fix and trusted OTA loader/boot header retained.')
m['bend_v1861']={'entry':'FX hold >=700 ms','k1':'temporary upward bend with automatic release','k2_targets':['1/4','1/2','FULL','1 1/2','DOUBLE'],'k3_curves':['LINEAR','SOFT','HARD','S-CURVE','EXP'],'curve_alters':'K1 response for both attack and return; not bend maximum','initial_curve':'LINEAR (exact V1.86 behavior)','store_persistent':False,'audio_path':'pre FX oscillator pitch multiplier','sprites_original_unchanged':True,'physical_fm1_verified':False}
warning='V1.86.1: K3 curve response is cross-built and unit/PCM-tested, but the encoder feel, CRT UI readability, real hardware DMA margin and post-USB-C power-on pop require physical FM1 validation. The K3 curve is performance/session state, not saved with a project.'
if warning not in m.setdefault('limitations',[]):m['limitations'].append(warning)
manifest.write_text(json.dumps(m,ensure_ascii=False,indent=2)+'\n')
installer=root/'WebInstaller.html';s=installer.read_text()
assert 'V1.86' in s and 'Stop at the FIRST NUL' in s and 'read-only identity queries' in s
# byte-preserve OTA+connection engine script except for the version in its user-facing log strings
pre_engine=re.findall(r'<script\b[^>]*>([\s\S]*?)</script>',s)[1]
s=s.replace('V1.86','V1.86.1') if 'V1.86.1' not in s else s
s,n=re.subn(r'(<script id="firmware"[^>]*>).*?(</script>)',lambda mt:mt.group(1)+base64.b64encode(pack).decode('ascii')+mt.group(2),s,count=1,flags=re.S)
assert n==1
s,n=re.subn(r"const PKG_HASH='[0-9a-f]{64}'",f"const PKG_HASH='{sha(pack)}'",s)
assert n==1
s,n=re.subn(r'const PKG_LEN=\d+',f'const PKG_LEN={len(pack)}',s)
assert n==1
assert 'Stop at the FIRST NUL' in s and 'read-only identity queries' in s
# Verify script identity ignoring version display text.  No OTA/protocol logic changes.
post_engine=re.findall(r'<script\b[^>]*>([\s\S]*?)</script>',s)[1]
assert post_engine.replace('V1.86.1','V1.86')==pre_engine.replace('V1.86.1','V1.86')
assert base64.b64decode(re.search(r'<script id="firmware"[^>]*>(.*?)</script>',s,re.S).group(1))==pack
installer.write_text(s)
print('FWSC',file.name,len(pack),sha(pack))
print('APP',len(app),sha(app),'free',581564-len(app))
print('OTA loader unchanged:',len(e1['ota.bin']),sha(e1['ota.bin']))
print('Boot header unchanged:',len(flash[:0x4000]),sha(flash[:0x4000]))
print('WebInstaller: same corrected connection/OTA scripts and matching new firmware')
