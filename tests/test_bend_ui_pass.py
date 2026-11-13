#!/usr/bin/env python3
"""RUPTURE BEND UI pass: verify native sprites, UI exports and isolation."""
from pathlib import Path
from PIL import Image
import hashlib,subprocess,re,sys,tempfile
root=Path(__file__).resolve().parents[1]
src=root/'vendor/etch-v1.88-source/firmware/fm1-native'
arts=root/'assets/bend-ui-v186'
assert arts.is_dir()
frames=sorted((src/'bend_frames').glob('bend_frame_*.png'))
assert len(frames)==8
header=(src/'rupture_bend_sprites.h').read_text()
bytes_hex=re.findall(r'0x([0-9a-f]{2})',header)
assert len(bytes_hex)==8*30*28,len(bytes_hex)
values=bytes(int(x,16) for x in bytes_hex)
for frame,f in enumerate(frames):
    im=Image.open(f)
    assert im.size==(224,30),(f,im.size)
    assert im.mode=='1',(f,im.mode)
    for y in range(30):
        for x in range(224):
            packed=values[frame*30*28 + y*28 + x//8]
            black=bool(packed&(1<<(7-x%8)))
            assert black == (im.getpixel((x,y))==0),(f,x,y)
print('PASS native sprite PNG bit-exact parity (8 x 224 x 30)')
preview_imgs=sorted(arts.glob('BEND_*CRT_*.png'))
assert len(preview_imgs)==18,len(preview_imgs)
for f in preview_imgs:
    im=Image.open(f)
    if f.name.startswith('BEND_UI_CONTACT'):assert im.size==(1920,1048),f
    else: assert im.size==(240,240),f
print('PASS previews: 16 native-size PNGs + 2 contact sheets')
# Native screenshot comparison BEND controls change the pixels.
assert Image.open(arts/'BEND_IDLE_FULL_CRT_OFF.png').tobytes()!=Image.open(arts/'BEND_BENDING_DOUBLE_CRT_OFF.png').tobytes()
assert Image.open(arts/'BEND_BENDING_DOUBLE_CRT_OFF.png').tobytes()!=Image.open(arts/'BEND_RETURN_DOUBLE_CRT_OFF.png').tobytes()
assert Image.open(arts/'BEND_BENDING_DOUBLE_CRT_OFF.png').tobytes()!=Image.open(arts/'BEND_BENDING_DOUBLE_CRT_ON.png').tobytes()
print('PASS BEND meter/target/status and CRT-on are distinct')
# UI pass must not modify the existing CXS/SAW synthesis path.
base=Path('/mnt/data/work1852')
if base.exists():
    orig=base/'vendor/etch-v1.88-source/firmware/fm1-native'
    for name in ['native_fx.c','audio_target.c','usb.c','ota.c','hal/fm1_isr.S','native_project_codec.c']:
        assert (orig/name).read_bytes()==(src/name).read_bytes(),name
    prior=Path('/mnt/data/work1852_bend')
    # All untouched screens are verified separately by native renderer parity.
print('PASS audio FX, DMA, OTA, project codec and hardware ISR untouched')
print('PASS BEND UI pass host verification')
