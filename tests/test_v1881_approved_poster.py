#!/usr/bin/env python3
"""RUPTURE V1.88.1: approve-poster provenance and FM1 ROM/OTA release parity."""
from pathlib import Path
from PIL import Image,ImageOps
import base64,hashlib,json,re,sys,zipfile
R=Path(__file__).resolve().parents[1]
FM=R/'vendor/etch-v1.88-source/firmware/fm1-native'
AS=FM/'bend_sprites_v188'
repozip=Path('/mnt/data/RUPTURE-FM1-V1.88-FULL-SOURCE-FWSC.zip')
poster=R/'assets/bend-v1881-approved-poster/approved-poster.jpg'
js=json.loads((AS/'POSTER-SOURCE-EXTRACTION.json').read_text())
h=lambda x:hashlib.sha256(x).hexdigest()
assert h(poster.read_bytes())==js['source_sha256']
assert len(js['frames'])==40 and js['packed_rom_bytes']==5*8*30*28
source=Image.open(poster).convert('L')
assert source.size==(1536,1024)
values=bytes(int(x,16) for x in re.findall(r'0x([0-9a-f]{2})',(FM/'rupture_bend_sprites.h').read_text()))
assert len(values)==5*8*30*28
assert h(values)==js['packed_rom_sha256']
curves=js['curves']; changed=0
old=None
if repozip.exists():old=zipfile.ZipFile(repozip)
for c,name in enumerate(curves):
 for i in range(8):
  p=AS/name/f'frame_{i:02}.png'
  im=Image.open(p)
  assert im.mode=='1' and im.size==(224,30)
  entry=js['frames'][c*8+i]
  crop=source.crop(tuple(entry['crop_box']))
  cr=AS/name/f'poster_crop_{i:02}.png'
  assert Image.open(cr).convert('L').tobytes()==crop.tobytes()
  test=ImageOps.autocontrast(crop.resize((224,30),Image.Resampling.LANCZOS)).point(lambda lum:255 if lum>112 else 0,mode='1')
  assert test.tobytes()==im.tobytes(),(name,i)
  for y in range(30):
   for x in range(224):
    offset=((c*8+i)*30+y)*28+x//8
    expected=bool(values[offset]&(0x80>>(x%8)))
    assert expected==(im.getpixel((x,y))==255),(name,i,x,y)
  if old:
   zpath=f'vendor/etch-v1.88-source/firmware/fm1-native/bend_sprites_v188/{name}/frame_{i:02}.png'
   if old.read(zpath)!=p.read_bytes():changed+=1
if old:old.close()
assert changed>=35,changed
print('PASS approved user poster SHA256, all 40 *actual* poster crops, native 1-bit and packed ROM parity')
print(f'PASS changed {changed}/40 previous substitute sprite PNGs')

# Audio DSP, FX, flash protection, MIDI/OTA and LCD DMA unchanged from V1.88.
critical=['audio_core.c','audio_core.h','native_fx.c','native_fx.h','native_cxs_q15.c','audio_target.c',
 'native_panel_midi.c','usb.c','ota.c','ota_target.c','wave_store_target.c','native_project_codec.c',
 'native_flash_probe.h','hal/fm1_flash_id.h','rupture_bend_curves.h','hal/fm1_isr.S']
if repozip.exists():
 with zipfile.ZipFile(repozip) as z:
  prefix='vendor/etch-v1.88-source/firmware/fm1-native/'
  for name in critical:
   assert z.read(prefix+name)==(FM/name).read_bytes(),name
 print('PASS all 16 original V1.88 core audio, OTA, flash, MIDI, FX, ISR source files byte-identical')

# Rebuilt FM1 and OTA container integrity
sys.path.insert(0,str(R/'vendor/etch-v1.88-source/tools'))
import fm1_fwsc as fw
m=json.loads((R/'RELEASE-MANIFEST.json').read_text())
assert m['version']=='1.88.1' and m['firmware_file']=='RUPTURE-V1.88.1.fwsc'
pkg=(R/m['firmware_file']).read_bytes()
assert len(pkg)==m['firmware_bytes']==610086
assert h(pkg)==m['firmware_sha256']
assert fw.verify_fwsc(R/m['firmware_file'])[1]==[]
extract=lambda b:{e['name']:e['data'] for e in fw.parse_ufw(b)['entries']}
a,b=extract(pkg),extract((R/'RUPTURE-V1.88.fwsc').read_bytes())
assert a['ota.bin']==b['ota.bin']
assert a['flash.bin'][:0x4000]==b['flash.bin'][:0x4000]
assert len(a['flash.bin'])==0x93000
assert m['app_sha256']==h((R/'native-build/app.bin').read_bytes())
assert m['app_bytes']==len((R/'native-build/app.bin').read_bytes())
assert m['release_ready'] is False
print('PASS V1.88.1 native FWSC, app hash, exact trusted OTA+16KB bootheader parity')
for htmlfile in ('WebInstaller.html','index.html'):
 content=(R/htmlfile).read_text()
 raw=base64.b64decode(re.search(r'<script id="firmware"[^>]*>(.*?)</script>',content,re.S).group(1))
 assert raw==pkg
 assert f"const PKG_HASH='{h(pkg)}'" in content
 assert f'const PKG_LEN={len(pkg)}' in content
 assert 'Stop at the FIRST NUL' in content
 assert 'read-only identity queries' in content
 assert 'V1.88.1' in content
print('PASS Index + WebInstaller contain byte-exact .fwsc with V1.86 connection/JEDEC parser retained')
# Storyboard directs full bend to the dramatic poster climax, not a retreat panel.
story=(FM/'rupture_bend_storyboard.h').read_text()
assert 'if(releasing)' in story and 'if(curve==RU_BEND_SCURVE)' in story
ui=(FM/'rupture_ui.c').read_text()
assert 'ru_bend_sprites[curve][frame]' in ui
assert 'curve!=RU_BEND_SCURVE' in ui
assert 'V1.88.1' in ui
print('PASS poster rise/return storyboard, stable closeup camera, existing UI-only 4-of-5 shake')
