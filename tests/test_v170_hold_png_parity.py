#!/usr/bin/env python3
"""Verify V1.88.3 ENV + preserved V1.70 LFO ROM images against PNG files."""
from pathlib import Path
from PIL import Image
import json,hashlib,re
ROOT=Path(__file__).resolve().parents[1]
R=ROOT/'assets/panel-hold-v170'
ENV=ROOT/'assets/panel-hold-v1883/env'
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
  if kind=='lfo':assert hashlib.sha256(packed).hexdigest()==manifest['sha256'][kind][i]
  # Existing LFO loop remains pixel-identical in its outer border.
  if kind=='lfo':
   orig=Image.open(ART/kind/'13.png').convert('1')
   for y in range(30):
    for x in [*range(0,64),*range(166,224)]:
     assert im.getpixel((x,y))==orig.getpixel((x,y)),(kind,i,x,y)
  if kind=='env':
   # The settling overlay must be the exact final 8 frames of the full bank.
   full=Image.open(ROOT/'assets/panel-animations-v1883/env'/f'{i+13:02}.png').convert('1')
   assert im.tobytes()==full.tobytes()
# Important: no existing original 20 sprite assets were modified by the generator.
assert len(list((ART/'env').glob('[0-9][0-9].png')))==20
assert len(list((ART/'lfo').glob('[0-9][0-9].png')))==20
print('PASS V1.88.3: 8 corrected ENV hold sprites + 8 original LFO sprites exact; permanent hole')
