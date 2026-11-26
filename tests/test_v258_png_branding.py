#!/usr/bin/env python3
"""Exact uploaded PNG is the only source for BOTH installed boot portraits."""
from pathlib import Path
import hashlib
import re
from PIL import Image

root=Path(__file__).resolve().parents[1]
source=root/'assets/boot-sikkenworks-photo-v258.png'
preview=root/'assets/boot-sikkenworks-photo-v258-display-240.png'
header=root/'vendor/etch-v1.88-source/firmware/fm1-native/rupture_sikkenworks_boot_logo.h'
assert hashlib.sha256(source.read_bytes()).hexdigest()==(
 '85d690dd543a5fdd1d3e1d0267760a37148b9a33b536b24560ef1a46966c0dff')
original=Image.open(source)
assert original.size==(1402,1122)
assert original.getchannel('A').getbbox()==(117,0,1290,1122)
assert preview.exists() and Image.open(preview).size==(240,240)
payload=header.read_text().split('ru_sikkenworks_logo_gray[RU_SIKKENWORKS_LOGO_BYTES]={',1)[1].split('};',1)[0]
packed=bytes(int(v,16) for v in re.findall(r'0x[0-9a-f]{2}',payload))
assert len(packed)==57600
frame=Image.open(preview)
assert frame.mode=='L' and frame.tobytes()==packed
assert len(set(packed))>=200, 'The original image has far more than 2 tones'
generator=(root/'tools/generate_v258_intro_logo.py').read_text()
assert 'Image.Resampling.NEAREST' in generator
assert 'Image.new(\'1\'' not in generator
assert 'pattern = (' not in generator
bringup=(root/'vendor/etch-v1.88-source/firmware/fm1-native/bringup.c').read_text()
assert 'ru_intro_gray_pending' in bringup
assert 'ru_sikkenworks_logo_gray[pos]' in bringup
assert 'lcd_blit(0,band*48u,240u,48u,output)' in bringup
assert 'ru_gray_logo_last!=ru_intro_gray_pending' in bringup
for name in ('app.bin','cpu1-fx-app.bin'):
 app=root/'native-build'/name
 if app.is_file():
  assert packed in app.read_bytes(),f'{name} does not contain the original 8-bit portrait'
print('PASS V2.0.58: supplied PNG SHA, 242-level nearest-neighbor grayscale pixels, NO dithering, RGB565 LCD and both apps exact')
