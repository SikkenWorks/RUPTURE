#!/usr/bin/env python3
"""Package V1.88.1 poster-exact BEND correction, retaining trusted OTA assets."""
from pathlib import Path
import base64, hashlib, json, re, sys, shutil
R=Path(__file__).resolve().parents[1]
N=R/'vendor/etch-v1.88-source'
appdir=N/'build/fm1-native-storage-updater-uac48-p05'
app=(appdir/'app.bin').read_bytes()
sha=lambda v:hashlib.sha256(v).hexdigest()
sys.path.insert(0,str(N/'tools'))
import fm1_fwsc as fw
base=R/'RUPTURE-V1.86.1.fwsc'
prior=R/'RUPTURE-V1.88.fwsc'
assert fw.verify_fwsc(base)[1]==[]
assert fw.verify_fwsc(prior)[1]==[]
assert not fw.validate_app_binary(app)
assets=fw.extract_stock_assets(base)
assert assets['product']=='FM-1_968'
prior_entries={v['name']:v['data'] for v in fw.parse_ufw(prior.read_bytes())['entries']}
flash=fw.build_flash_image_from_assets(app,assets['uboot'],assets['cfg_tool'],assets['eq_cfg'],assets['key'],isd_config=assets['isd_config'],flash_header_payload=assets['flash_header_payload'])
assert len(flash)==len(prior_entries['flash.bin'])==0x93000
assert flash[:0x4000]==prior_entries['flash.bin'][:0x4000]
pack=fw.build_fwsc(flash,assets['ota'],'FM-1_968')
file=R/'RUPTURE-V1.88.1.fwsc';file.write_bytes(pack)
assert fw.verify_fwsc(file)[1]==[]
new_entries={v['name']:v['data'] for v in fw.parse_ufw(pack)['entries']}
assert new_entries['ota.bin']==prior_entries['ota.bin']
assert new_entries['flash.bin'][:0x4000]==prior_entries['flash.bin'][:0x4000]
assert new_entries['flash.bin']==flash
for name in ('app.bin','app.bin.audit.json','native-build-options.json'):
 shutil.copy2(appdir/name,R/'native-build'/name)
opts=json.loads((R/'native-build/native-build-options.json').read_text())
assert opts['version']=='RUPTURE V1.88.1' and opts['app_sha256']==sha(app)
mfile=R/'RELEASE-MANIFEST.json';m=json.loads(mfile.read_text())
m.update(version='1.88.1',firmware_file=file.name,firmware_bytes=len(pack),firmware_sha256=sha(pack),app_bytes=len(app),app_sha256=sha(app),physical_fm1_verified=False,hardware_verified=False,release_ready=False,release_stage='SOFTWARE_RC_PENDING_HARDWARE_VALIDATION',release_feature='Approved V1.88 poster source cells in all five K3 BEND animations',release_note='V1.88.1 corrects the visual mismatch: the approved 5x8 poster frame cells now become native 1-bit ROM sprites. Eruptions use rise/return storyboard; S-CURVE camera remains stable. All audio / FX / flash / OTA code retained; physical test pending.')
source=(N/'firmware/fm1-native/bend_sprites_v188/POSTER-SOURCE-EXTRACTION.json')
extract=json.loads(source.read_text())
m['bend_v1881']={'source':'user-approved poster JPEG','poster_sha256':extract['source_sha256'],'poster_resolution':[1536,1024],'curves':extract['curves'],'frames_per_curve':8,'total_frames':40,'frame_width':224,'frame_height':30,'rom_bytes':33600,'rom_sha256':extract['packed_rom_sha256'],'user_provided_source_crops':True,'no_new_audio_isr_work':True,'ui_only_rise_return_mapping':True,'four_of_five_shake':True,'ship_closeup_shake':False,'original_poster_resampled':True,'physical_fm1_verified':False}
mfile.write_text(json.dumps(m,ensure_ascii=False,indent=2)+'\n')
installer=R/'WebInstaller.html';s=(R/'reference/RUPTURE-WebInstaller-V1.88.html').read_text()
assert 'V1.88' in s and 'V1.88.1' not in s
assert 'Stop at the FIRST NUL' in s and 'read-only identity queries' in s
ota_script_before=re.findall(r'<script\b[^>]*>([\s\S]*?)</script>',s)[1]
s=s.replace('V1.88','V1.88.1')
s, num=re.subn(r'(<script id="firmware"[^>]*>).*?(</script>)',lambda v:v.group(1)+base64.b64encode(pack).decode('ascii')+v.group(2),s,count=1,flags=re.S)
assert num==1
s, num=re.subn(r"const PKG_HASH='[0-9a-f]{64}'",f"const PKG_HASH='{sha(pack)}'",s);assert num==1
s, num=re.subn(r'const PKG_LEN=\d+',f'const PKG_LEN={len(pack)}',s);assert num==1
assert re.findall(r'<script\b[^>]*>([\s\S]*?)</script>',s)[1].replace('V1.88.1','V1.88')==ota_script_before
assert base64.b64decode(re.search(r'<script id="firmware"[^>]*>(.*?)</script>',s,re.S).group(1))==pack
assert 'Stop at the FIRST NUL' in s and 'read-only identity queries' in s
installer.write_text(s)
print('PASS app',len(app),'headroom',581564-len(app),'SHA256',sha(app))
print('PASS firmware',len(pack),'SHA256',sha(pack),'product FM-1_968')
print('PASS unchanged OTA',sha(new_entries['ota.bin']))
print('PASS unchanged bootheader',sha(flash[:0x4000]))
print('PASS WebInstaller embedded FWSC and read-only probe/identity handling retained')
