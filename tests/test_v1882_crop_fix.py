#!/usr/bin/env python3
"""RUPTURE V1.88.2: no-stretch BEND frames and OTA/audio isolation checks."""
from pathlib import Path
from PIL import Image
import base64,hashlib,json,re,sys
R=Path(__file__).resolve().parents[1]
P=Path('/mnt/data/rupture-v188-poster-fix')
FM=R/'vendor/etch-v1.88-source/firmware/fm1-native'
DB=FM/'bend_sprites_v188'
poster=Image.open(R/'assets/bend-v1881-approved-poster/approved-poster.jpg').convert('L')
info=json.loads((DB/'POSTER-SOURCE-EXTRACTION.json').read_text())
assert info['resampling']=='none' and info['scale_x']==info['scale_y']==1
assert len(info['frames'])==40 and info['packed_rom_bytes']==33600
values=bytes(int(v,16) for v in re.findall(r'0x([0-9a-f]{2})',(FM/'rupture_bend_sprites.h').read_text()))
assert len(values)==33600 and hashlib.sha256(values).hexdigest()==info['packed_rom_sha256']
changed=0
for j,entry in enumerate(info['frames']):
 name=entry['curve'];fi=entry['frame']
 orig=poster.crop(tuple(entry['original_cell_box']))
 sub=orig.crop(tuple(entry['source_crop_box_in_cell']))
 assert sub.width==orig.width and sub.height==30
 assert (DB/name/f'poster_crop_{fi:02}.png').read_bytes()
 assert Image.open(DB/name/f'source_camera_crop_{fi:02}.png').convert('L').tobytes()==sub.tobytes()
 x0=entry['on_screen_x']
 back=Image.new('L',(224,30),0);back.paste(sub,(x0,0))
 native=back.point(lambda v:255 if v>112 else 0,mode='1')
 observed=Image.open(DB/name/f'frame_{fi:02}.png')
 assert observed.mode=='1' and observed.size==(224,30)
 assert observed.tobytes()==native.tobytes(),(name,fi)
 for y in range(30):
  for x in range(224):
   offset=j*30*28+y*28+x//8
   assert bool(values[offset]&(0x80>>(x%8)))==(observed.getpixel((x,y))==255)
 assert all(observed.getpixel((x,y))==0 for y in range(30) for x in (*range(x0),*range(x0+sub.width,224)))
 old=DB/name/f'frame_{fi:02}.png'
 if (P/old.relative_to(R)).read_bytes()!=old.read_bytes():changed+=1
assert changed==40,changed
print('PASS 40/40 original source camera crops are 1:1, no distortion, exact ROM/PNG parity')
print('PASS 40/40 corrected images differ from distorted V1.88.1; black side margins correct')
# No changes to any core audio, midi, effects, storage or OTA source.
critical=['audio_core.c','audio_core.h','native_fx.c','native_fx.h','native_cxs_q15.c','audio_target.c',
 'native_panel_midi.c','usb.c','ota.c','ota_target.c','wave_store_target.c','native_project_codec.c',
 'native_flash_probe.h','hal/fm1_flash_id.h','rupture_bend_curves.h','hal/fm1_isr.S',
 'bringup.c','rupture_bend_storyboard.h']
for name in critical:
 assert (FM/name).read_bytes()==(P/'vendor/etch-v1.88-source/firmware/fm1-native'/name).read_bytes(),name
ui_new=(FM/'rupture_ui.c').read_text();ui_old=(P/'vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c').read_text()
assert ui_new.replace('"V1.88.2"','"V1.88.1"')==ui_old,'Only the version label should differ in UI C source'
assert 'ru_bend_sprites[curve][frame]' in ui_new and 'curve!=RU_BEND_SCURVE' in ui_new
print('PASS untouched audio, MIDI, effects, flash storage, OTA, UI runtime and storyboard sources')

sys.path.insert(0,str(R/'vendor/etch-v1.88-source/tools'))
import fm1_fwsc as fw
sha=lambda b:hashlib.sha256(b).hexdigest()
m=json.loads((R/'RELEASE-MANIFEST.json').read_text())
assert m['version']=='1.88.2' and m['firmware_file']=='RUPTURE-V1.88.2.fwsc'
package=(R/m['firmware_file']).read_bytes()
assert len(package)==610086==m['firmware_bytes'] and sha(package)==m['firmware_sha256']
assert fw.verify_fwsc(R/m['firmware_file'])[1]==[]
extract=lambda b:{e['name']:e['data'] for e in fw.parse_ufw(b)['entries']}
new=extract(package); old=extract((R/'RUPTURE-V1.88.1.fwsc').read_bytes())
assert new['ota.bin']==old['ota.bin']
assert new['flash.bin'][:0x4000]==old['flash.bin'][:0x4000]
assert m['app_sha256']==sha((R/'native-build/app.bin').read_bytes())
for html in ('WebInstaller.html','index.html'):
 src=(R/html).read_text()
 b64=re.search(r'<script id="firmware"[^>]*>(.*?)</script>',src,re.S).group(1)
 assert base64.b64decode(b64)==package
 assert f"const PKG_HASH='{sha(package)}'" in src
 assert f'const PKG_LEN={len(package)}' in src
 assert 'Stop at the FIRST NUL' in src and 'read-only identity queries' in src
print('PASS FM1 app, FWSC CRC, native OTA loader, protected bootheader, WebInstaller and index exact parity')
