#!/usr/bin/env python3
"""RUPTURE V1.88.2 comparison: 1:1 poster pixels and their cropped FM1 frame."""
from pathlib import Path
from PIL import Image,ImageDraw
import json
R=Path(__file__).resolve().parents[1]
A=R/'vendor/etch-v1.88-source/firmware/fm1-native/bend_sprites_v188'
O=R/'assets/bend-v1882-crop-ui'
O.mkdir(parents=True,exist_ok=True)
meta=json.loads((A/'POSTER-SOURCE-EXTRACTION.json').read_text())
labels=('LINEAR / EVEREST','SOFT / MONOLITH','HARD / PIRATE SHIP','S-CURVE / SHIP CAMERA','EXP / KRAKEN')
CW, RH=224,170
im=Image.new('RGB',(8*CW,36+5*RH),(8,8,8))
d=ImageDraw.Draw(im)
d.text((9,11),'V1.88.2  |  WHITE CAMERA BOX: 156x30 SOURCE PIXELS  |  BOTTOM: 224x30 NATIVE (NO STRETCH)',fill='white')
for ci,name in enumerate(meta['curves']):
  ybase=36+ci*RH
  d.text((4,ybase),labels[ci],fill='white')
  for fi in range(8):
    x=fi*CW
    src=Image.open(A/name/f'poster_crop_{fi:02}.png').convert('RGB')
    assert src.width<=CW
    top=ybase+14
    off=(CW-src.width)//2
    im.paste(src,(x+off,top))
    data=meta['frames'][ci*8+fi]
    box=data['source_crop_box_in_cell']
    # White horizontal lines reveal the chosen literal 30px-high source crop.
    d.rectangle((x+off,top+box[1],x+off+src.width-1,top+box[3]-1),outline=(255,255,255),width=1)
    native=Image.open(A/name/f'frame_{fi:02}.png').convert('RGB')
    im.paste(native,(x,ybase+112))
    d.text((x+5,ybase+145),f'{fi+1}/8  CROP Y={box[1]}',fill=(225,225,225))
    d.rectangle((x,ybase+13,x+CW-1,ybase+144),outline=(65,65,65))
p=O/'BEND-V1.88.2-ORIGINAL-1TO1-CROPS-VS-FM1-40FRAMES.png'
im.save(p,optimize=True)
print('Wrote 40-frame source window vs native 1:1 crop proof',p)
