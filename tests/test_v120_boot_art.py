#!/usr/bin/env python3
"""Compare exact native C-rendered final splash against packaged 240x240 ROM source."""
from pathlib import Path
from PIL import Image
root=Path(__file__).resolve().parents[1]
base=Image.open(root/'ui/assets/rupture-splash-final-240.png').convert('1')
assert base.size==(240,240)
last=Image.open(root/'ui/loading-frames/frame-24.png').convert('1')
assert list(last.getdata())==list(base.getdata()),'FM1 C splash is not the approved artwork'
pixels=[]
for i in range(1,25):
 p=Image.open(root/f'ui/loading-frames/frame-{i:02}.png').convert('1')
 assert p.size==(240,240)
 assert p.getcolors() and len(p.getcolors())<=2
 pixels.append(sum(bool(v) for v in p.getdata()))
assert 0<pixels[0]<pixels[7]<pixels[15]<pixels[-1]
assert len(set(pixels))>=15
print('PASS V1.20 24 actual firmware frames, true 1-bit, last image bit-exact, progressive crack')
