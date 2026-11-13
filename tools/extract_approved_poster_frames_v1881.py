#!/usr/bin/env python3
"""Reproducible RUPTURE V1.88.2 BEND animation crop correction.

The approved 1536x1024 poster has 5 rows x 8 illustrated cells. The 40
cells are cropped exactly as in V1.88.1, then a 156px-wide by 30px-high
subject-tracking **camera window** is taken with no resampling at all. That
unaltered 1:1 pixel crop is centered in the 224x30 RUPTURE viewport with
black side margins. Only final 1-bit conversion happens before ROM packing.

The old, incorrect 156x90 -> 224x30 independent X/Y resize is prohibited.
The UI layout, frame count, shake behavior, effects and audio callback are not
modified by this script.
"""
from pathlib import Path
from PIL import Image
import hashlib, json

ROOT=Path(__file__).resolve().parents[1]
SOURCE=ROOT/'assets/bend-v1881-approved-poster/approved-poster.jpg'
FM=ROOT/'vendor/etch-v1.88-source/firmware/fm1-native'
OUT=FM/'bend_sprites_v188'
W,H=224,30
CURVES=('linear-everest','soft-monolith','hard-pirate-ship','s-curve-ship-closeup','exp-kraken')
ROW_Y=((74,165),(182,279),(294,395),(408,507),(523,618))
COLUMN_X=(241,401,561,721,881,1041,1201,1361,1520)
THRESHOLD=112
# Camera Y for every single 30px-high crop, measured relative to original cell.
# Eruptions follow their subjects upwards then down; S-CURVE pans upward with
# the ship (and continues to obey its pre-existing no-shake rule).
CAMERA_Y_CENTER=(
 (75,72,62,48,32,32,50,73),
 (74,70,61,55,46,43,50,72),
 (75,68,57,50,40,40,54,70),
 (72,67,62,52,45,38,38,36),
 (75,70,62,52,42,40,52,72),
)

def main():
    assert SOURCE.is_file(), SOURCE
    poster=Image.open(SOURCE).convert('L')
    assert poster.size==(1536,1024)
    OUT.mkdir(parents=True,exist_ok=True)
    payload=bytearray()
    data={
      'source':'user-approved poster JPEG, extracted frame cells',
      'source_sha256':hashlib.sha256(SOURCE.read_bytes()).hexdigest(),
      'source_dimensions':list(poster.size),'version':'V1.88.2 crop fix',
      'curves':list(CURVES),'native_dimensions':[W,H],
      'conversion':'crop to exact source pixels; 1:1 aspect; center on black 224x30 canvas; threshold 112; NO image resize',
      'resampling':'none','scale_x':1,'scale_y':1,
      'camera_center_y':[list(r) for r in CAMERA_Y_CENTER],
      'packed_bytes_expected':5*8*30*28,'frames':[]}
    lines=[
      '/* RUPTURE V1.88.2: original approved poster cropped 1:1, no stretch.',
      ' * 5 x 8 native 224x30 1-bit frames; black margin preserves aspect.',
      ' * Bit 1 = white. Prepacked ROM, UI-only blit; no audio/ISR work.',
      ' * SPDX-License-Identifier: GPL-3.0-only */',
      '#ifndef RUPTURE_BEND_SPRITES_H','#define RUPTURE_BEND_SPRITES_H',
      '#include <stdint.h>','#define RU_BEND_CURVES 5u','#define RU_BEND_FRAMES 8u',
      '#define RU_BEND_WIDTH 224u','#define RU_BEND_HEIGHT 30u','#define RU_BEND_BYTES 28u',
      '#define RU_BEND_X 8u','#define RU_BEND_Y 50u',
      'static const uint8_t ru_bend_sprites[RU_BEND_CURVES][RU_BEND_FRAMES][RU_BEND_HEIGHT][RU_BEND_BYTES]={']
    for ci,name in enumerate(CURVES):
      dest=OUT/name;dest.mkdir(parents=True,exist_ok=True)
      sheet=Image.new('1',(W*8,H),0)
      lines.append(f' {{ /* {name}: per-frame source crop */')
      for fi in range(8):
        box=(COLUMN_X[fi]+2,ROW_Y[ci][0],COLUMN_X[fi+1]-2,ROW_Y[ci][1])
        orig=poster.crop(box)
        orig.save(dest/f'poster_crop_{fi:02}.png')
        assert orig.width<=W and orig.height>=H,(ci,fi,orig.size)
        y0=max(0,min(orig.height-H,CAMERA_Y_CENTER[ci][fi]-H//2))
        src_box=(0,y0,orig.width,y0+H)
        # Image.crop preserves original pixel coordinates, with *no* scaling.
        unscaled=orig.crop(src_box)
        unscaled.save(dest/f'source_camera_crop_{fi:02}.png')
        result=Image.new('L',(W,H),0)
        x0=(W-unscaled.width)//2
        result.paste(unscaled,(x0,0))
        native=result.point(lambda v:255 if v>THRESHOLD else 0,mode='1')
        native.save(dest/f'frame_{fi:02}.png')
        sheet.paste(native,(fi*W,0))
        lines.append('  {')
        for y in range(H):
          row=[]
          for bx in range(W//8):
            v=0
            for bit in range(8):
              if native.getpixel((bx*8+bit,y))==255: v|=0x80>>bit
            payload.append(v);row.append(f'0x{v:02x}')
          lines.append('   {'+','.join(row)+'}'+(',' if y<H-1 else ''))
        lines.append('  }'+(',' if fi<7 else ''))
        data['frames'].append({'curve':name,'frame':fi,'original_cell_box':list(box),
          'source_crop_box_in_cell':list(src_box),'on_screen_x':x0,
          'source_sha256':hashlib.sha256(unscaled.tobytes()).hexdigest(),
          'native_sha256':hashlib.sha256(native.tobytes()).hexdigest()})
      sheet.save(dest/'contact.png')
      sheet.resize((W*8*2,H*3),Image.Resampling.NEAREST).save(dest/'contact_2x.png')
      lines.append(' }'+(',' if ci<4 else ''))
    lines+=['};','#endif']
    (FM/'rupture_bend_sprites.h').write_text('\n'.join(lines)+'\n')
    assert len(payload)==33600
    data['packed_rom_sha256']=hashlib.sha256(payload).hexdigest()
    data['packed_rom_bytes']=len(payload)
    (OUT/'POSTER-SOURCE-EXTRACTION.json').write_text(json.dumps(data,indent=2)+'\n')
    overview=Image.new('1',(W*8,H*5),0)
    for ci,name in enumerate(CURVES):
      overview.paste(Image.open(OUT/name/'contact.png'),(0,ci*H))
    overview.save(OUT/'BEND-V1.88.2-CROPPED-40-FRAMES.png')
    overview.resize((W*8,H*5*3),Image.Resampling.NEAREST).save(OUT/'BEND-V1.88.2-CROPPED-40-FRAMES-3X.png')
    print('PASS: 5x8 frames from unscaled 1:1 source crops; ROM bytes',len(payload))
    print('PASS: no X/Y resize; source sha256',data['source_sha256'])

if __name__=='__main__':main()
