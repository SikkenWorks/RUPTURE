#!/usr/bin/env python3
"""Side-by-side source poster crop and the actual 1-bit native FM1 frames."""
from PIL import Image, ImageDraw, ImageFont
from pathlib import Path
R=Path(__file__).resolve().parents[1]
A=R/'vendor/etch-v1.88-source/firmware/fm1-native/bend_sprites_v188'
P=R/'assets/bend-v1881-approved-poster'
P.mkdir(parents=True,exist_ok=True)
sets=[('LINEAR — EVEREST','linear-everest'),('SOFT — MONOLITH','soft-monolith'),('HARD — PIRATE SHIP','hard-pirate-ship'),('S-CURVE — SHIP CAMERA','s-curve-ship-closeup'),('EXP — KRAKEN','exp-kraken')]
cell_w=224
row_h=160
im=Image.new('RGB',(8*cell_w,30+row_h*5),(14,14,14))
d=ImageDraw.Draw(im)
d.text((8,7),'USER-APPROVED POSTER CROPS / FM1 224x30 1-BIT ROM SPRITES — RUPTURE V1.88.1',fill='white')
for ci,(name,folder) in enumerate(sets):
 y0=30+ci*row_h
 d.text((8,y0),name,fill='white')
 for j in range(8):
  crop=Image.open(A/folder/f'poster_crop_{j:02}.png').convert('L')
  # Show the source composition across 224x90 with no changes other than sizing.
  ref=crop.resize((cell_w,88),Image.Resampling.LANCZOS)
  native=Image.open(A/folder/f'frame_{j:02}.png').convert('RGB')
  im.paste(ref,(j*cell_w,y0+16))
  im.paste(native,(j*cell_w,y0+111))
  d.rectangle((j*cell_w,y0+15,(j+1)*cell_w-1,y0+142),outline=(90,90,90))
  d.text((j*cell_w+4,y0+143),f'FRAME {j+1:02}',fill=(225,225,225))
p=P/'POSTER-SOURCE-VS-NATIVE-FM1-ALL-40.png'
im.save(p,optimize=True)
print(p)
