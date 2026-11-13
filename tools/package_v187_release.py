#!/usr/bin/env python3
"""Package the FM1 V1.87 asset-only update with the already-audited V1.86.1 OTA assets."""
from pathlib import Path
import base64,hashlib,json,re,sys,shutil
root=Path(__file__).resolve().parents[1]
base=root/'RUPTURE-V1.86.1.fwsc'
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
file=root/'RUPTURE-V1.87.fwsc';file.write_bytes(pack)
assert fw.verify_fwsc(file)[1]==[]
e1={x['name']:x['data'] for x in fw.parse_ufw(pack)['entries']}
assert e1['ota.bin']==e0['ota.bin']
assert e1['flash.bin'][:0x4000]==e0['flash.bin'][:0x4000]
assert e1['flash.bin']==flash
for item in ('app.bin','app.bin.audit.json','native-build-options.json'):
 shutil.copy2(appdir/item,root/'native-build'/item)
b=json.loads((root/'native-build/native-build-options.json').read_text())
assert b['version']=='RUPTURE V1.87'
assert b['app_sha256']==sha(app)
manifest=root/'RELEASE-MANIFEST.json'
m=json.loads(manifest.read_text())
m.update(version='1.87',firmware_file=file.name,firmware_bytes=len(pack),firmware_sha256=sha(pack),app_bytes=len(app),app_sha256=sha(app),physical_fm1_verified=False,hardware_verified=False,release_ready=False,release_stage='SOFTWARE_RC_PENDING_HARDWARE_VALIDATION',release_feature='Everest-style eight-frame BEND mountain sprites (UI-only)',release_note='V1.87: replaces only the BEND-page 8 x 224x30 1-bit ROM mountain animation sprites; unchanged V1.86.1 K3 curves, CXS/SAW DSP and V1.86 connection/OTA fix; physical hardware validation pending.')
if 'bend_v1861' in m:
 m['bend_v1861']['sprites_original_unchanged']=False
 m['bend_v1861']['sprites_v187']='Eight offline-processed 224x30 monochrome Everest eruption frames; packed as ROM-only UI graphics'
m['bend_v187']={'sprites':8,'frame_width':224,'frame_height':30,'pixel_format':'1-bit','data_location':'read-only ROM','source':'approved generated mountain sheet','audio_isr_changes':False,'ota_changes':False,'curve_changes':False,'physical_fm1_verified':False}
warn='V1.87: More detailed Everest BEND sprite graphics are prepacked in ROM and require no extra DSP or audio ISR work, but FM1 LCD timings, UI refresh and actual real-time audio DMA stability require physical validation.'
if warn not in m.setdefault('limitations',[]):m['limitations'].append(warn)
manifest.write_text(json.dumps(m,ensure_ascii=False,indent=2)+'\n')

installer=root/'WebInstaller.html';s=installer.read_text()
assert 'V1.86.1' in s and 'Stop at the FIRST NUL' in s and 'read-only identity queries' in s
before=re.findall(r'<script\b[^>]*>([\s\S]*?)</script>',s)[1]
s=s.replace('V1.86.1','V1.87')
s,n=re.subn(r'(<script id="firmware"[^>]*>).*?(</script>)',lambda mt:mt.group(1)+base64.b64encode(pack).decode('ascii')+mt.group(2),s,count=1,flags=re.S)
assert n==1
s,n=re.subn(r"const PKG_HASH='[0-9a-f]{64}'",f"const PKG_HASH='{sha(pack)}'",s);assert n==1
s,n=re.subn(r'const PKG_LEN=\d+',f'const PKG_LEN={len(pack)}',s);assert n==1
assert 'Stop at the FIRST NUL' in s and 'read-only identity queries' in s
after=re.findall(r'<script\b[^>]*>([\s\S]*?)</script>',s)[1]
assert after.replace('V1.87','V1.86.1')==before
assert base64.b64decode(re.search(r'<script id="firmware"[^>]*>(.*?)</script>',s,re.S).group(1))==pack
installer.write_text(s)
print('FWSC',file.name,len(pack),sha(pack))
print('APP',len(app),sha(app),'free',581564-len(app))
print('OTA loader unchanged:',len(e1['ota.bin']),sha(e1['ota.bin']))
print('Boot header unchanged:',len(flash[:0x4000]),sha(flash[:0x4000]))
print('WebInstaller: unchanged corrected FM1 connection/OTA engine and matching V1.87 FWSC')
