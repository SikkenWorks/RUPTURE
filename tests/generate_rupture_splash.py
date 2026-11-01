#!/usr/bin/env python3
"""Generate deterministic read-only 1-bit FM1 splash ROM from approved 240x240 art.
No image decode at runtime; this file is for development/build reproducibility.
"""
from pathlib import Path
from PIL import Image
root=Path(__file__).resolve().parents[1]
image=root/'ui/assets/rupture-splash-final-240.png'
out=root/'vendor/etch-v1.88-source/firmware/fm1-native/rupture_splash_bitmap.h'
im=Image.open(image)
assert im.size==(240,240),im.size
pixels=im.convert('1')
bits=bytearray(7200)
for y in range(240):
 for x in range(240):
  if pixels.getpixel((x,y)):
   k=y*240+x
   bits[k>>3]|=128>>(k&7)
lines=['/* Auto-generated 240x240 pure 1-bit logo/tear, read-only flash; do not edit. */',
       '/* Source: ui/assets/rupture-splash-final-240.png */',
       '#ifndef RU_SPLASH_BITMAP_H','#define RU_SPLASH_BITMAP_H',
       'static const uint8_t ru_splash_final_bitmap[7200] = {']
for i in range(0,len(bits),20):
 lines.append(' '+', '.join('0x%02x'%v for v in bits[i:i+20])+(',' if i+20<len(bits) else ''))
lines+=['};','#endif']
out.write_text('\n'.join(lines)+'\n')
print(f'Wrote {out.name}: {len(bits)} ROM bytes, {sum(v.bit_count() for v in bits)} white pixels')
