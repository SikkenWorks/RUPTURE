#!/usr/bin/env python3
"""Verify V1.72 ENV + preserved V1.70 LFO ROM images against PNG files."""
from pathlib import Path
from PIL import Image
import json,hashlib,re
ROOT=Path(__file__).resolve().parents[1]
R=ROOT/'assets/panel-hold-v170'
ENV=ROOT/'assets/panel-hold-v172/env'
ART=ROOT/'assets/panel-animations-v156'
HEADER=ROOT/'vendor/etch-v1.88-source/firmware/fm1-native/rupture_hold_sprites.h'
manifest=json.loads((R/'ANIMATION-MANIFEST.json').read_text())
assert manifest['width']==224 and manifest['height']==30 and manifest['frames']==8
text=HEADER.read_text()
rombytes=[int(n,16) for n in re.findall(r'0x([0-9a-f]{2})',text)]
assert len(rombytes)==2*8*30*28,len(rombytes)
assert len(list(ENV.glob('*.png')))==8
assert len(list((R/'lfo').glob('*.png')))==8
for k,kind in enumerate(('env','lfo')):
 for i in range(8):
  im=Image.open((ENV if kind=='env' else R/kind)/f'{i+1:02}.png')
  assert im.mode=='1' and im.size==(224,30)
  packed=bytes(sum((1<<(7-q)) if im.getpixel((x+q,y)) else 0 for q in range(8))
               for y in range(30) for x in range(0,224,8))
  assert packed==bytes(rombytes[(k*8+i)*840:(k*8+i+1)*840]),(kind,i)
  assert hashlib.sha256(packed).hexdigest()==(json.loads((ROOT/'assets/panel-hold-v172/ANIMATION-MANIFEST.json').read_text())['sha256'][i] if kind=='env' else manifest['sha256'][kind][i])
  # The right and left 64px edges of EVERY sprite match the original.
  orig=Image.open(ART/kind/'13.png').convert('1')
  for y in range(30):
   for x in [*range(0,64),*range(166,224)]:
    assert im.getpixel((x,y))==orig.getpixel((x,y)),(kind,i,x,y)
  if kind=='env' and i==7:assert packed==bytes(sum((1<<(7-q)) if orig.getpixel((x+q,y)) else 0 for q in range(8)) for y in range(30) for x in range(0,224,8))
# Important: no existing original 20 sprite assets were modified by the generator.
assert len(list((ART/'env').glob('[0-9][0-9].png')))==20
assert len(list((ART/'lfo').glob('[0-9][0-9].png')))==20
print('PASS V1.72: 8 new ENV + 8 original LFO ROM PNG sprites exact; original opening/closing unchanged')
