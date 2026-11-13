#!/usr/bin/env python3
"""RUPTURE V1.88: new ROM sprites + preserved OTA/audio/flash + release parity."""
from pathlib import Path
from PIL import Image
import base64,hashlib,json,re,sys
R=Path(__file__).resolve().parents[1]
B=Path('/mnt/data/rupture-v187')
SRC='vendor/etch-v1.88-source/firmware/fm1-native/'
A=R/SRC
sys.path.insert(0,str(R/'vendor/etch-v1.88-source/tools'))
import fm1_fwsc as fw
sha=lambda b:hashlib.sha256(b).hexdigest()
manifest=json.loads((R/'RELEASE-MANIFEST.json').read_text())
assert manifest['version']=='1.88' and manifest['firmware_identity']=='FM-1_968'
fbytes=(R/'RUPTURE-V1.88.fwsc').read_bytes()
assert len(fbytes)==610086
assert len(fbytes)==manifest['firmware_bytes']
assert sha(fbytes)==manifest['firmware_sha256']
assert fw.verify_fwsc(R/'RUPTURE-V1.88.fwsc')[1]==[]
assert manifest['app_bytes']==len((R/'native-build/app.bin').read_bytes())
assert manifest['app_sha256']==sha((R/'native-build/app.bin').read_bytes())
extract=lambda b:{e['name']:e['data'] for e in fw.parse_ufw(b)['entries']}
a=extract(fbytes)
b=extract((R/'RUPTURE-V1.87.fwsc').read_bytes())
assert a['ota.bin']==b['ota.bin']
assert a['flash.bin'][:16384]==b['flash.bin'][:16384]
assert len(a['flash.bin'])==0x93000
print('PASS native V1.88 package, flash size, preserved OTA loader+16KB bootheader')

critical=['audio_core.c','audio_core.h','native_fx.c','native_fx.h','native_cxs_q15.c','audio_target.c',
 'native_panel_midi.c','usb.c','ota.c','ota_target.c','wave_store_target.c','native_project_codec.c',
 'native_flash_probe.h','hal/fm1_flash_id.h','rupture_bend_curves.h','hal/fm1_isr.S']
for name in critical:
 assert (A/name).read_bytes()==(B/SRC/name).read_bytes(), name
print('PASS 16 audio/Fx/MIDI/OTA/storage/flash/curve source files byte-identical to V1.87')

header=(A/'rupture_bend_sprites.h').read_text()
assert '#define RU_BEND_CURVES 5u' in header
assert '#define RU_BEND_FRAMES 8u' in header
assert '#define RU_BEND_HEIGHT 30u' in header
payload=bytes(int(x,16) for x in re.findall(r'0x([0-9a-f]{2})',header))
assert len(payload)==5*8*30*28,len(payload)
asset=A/'bend_sprites_v188'
names=['linear-everest','soft-monolith','hard-pirate-ship','s-curve-ship-closeup','exp-kraken']
for curve,name in enumerate(names):
 for frame in range(8):
  png=asset/name/f'frame_{frame:02}.png'
  im=Image.open(png)
  assert im.size==(224,30) and im.mode=='1',(png,im.size,im.mode)
  for y in range(30):
   for x in range(224):
    byte=payload[(((curve*8)+frame)*30+y)*28+x//8]
    foreground=bool(byte&(0x80>>(x%8)))
    assert foreground==(im.getpixel((x,y))==255),(curve,frame,x,y)
  if curve==0:
   original=Image.open(A/'bend_frames'/f'bend_frame_{frame:02}.png').convert('1')
   assert original.tobytes()==im.tobytes(),'Everest must be untouched'
assert len(payload)==33600
print('PASS 40 native 224x30 1-bit PNGs exactly equal 33600 bytes of packed ROM sprites')
print('PASS LINEAR Everest matches the original V1.87 artwork pixel-for-pixel')

ui=(A/'rupture_ui.c').read_text();controls=(A/'bringup.c').read_text()
assert 'ru_bend_sprites[curve][frame]' in ui
assert 'curve!=RU_BEND_SCURVE' in ui
assert 'ru_bend_last_visual_shake' in controls
assert 'ru_lcd_should_paint' in controls
assert '85u' in controls
assert '"V1.88"' in ui
assert "'version':'RUPTURE V1.88'" in (A/'build.py').read_text()
print('PASS K3 sprite-selection/shake + stable S-CURVE camera, existing LCD repaint gating')
for name in ['WebInstaller.html','index.html']:
 h=(R/name).read_text()
 assert base64.b64decode(re.search(r'<script id="firmware"[^>]*>(.*?)</script>',h,re.S).group(1))==fbytes
 assert f"const PKG_HASH='{sha(fbytes)}'" in h
 assert f'const PKG_LEN={len(fbytes)}' in h
 assert 'Stop at the FIRST NUL' in h and 'read-only identity queries' in h
 assert 'V1.88' in h
print('PASS WebInstaller + index embed exactly V1.88; retain V1.86 identity/connection fix')
assert not manifest['physical_fm1_verified'] and not manifest['release_ready']
print('PASS V1.88 release RC software validation; FM1 physical validation remains pending')
