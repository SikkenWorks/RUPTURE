#!/usr/bin/env python3
"""Extract the exact light modules from the user-supplied 2048px QR PNG.

No QR generation and no URL recoding. This preserves the author's visual
symbol and all 841 modules pixel-for-pixel at module granularity.
"""
from pathlib import Path
from PIL import Image
import hashlib

root=Path(__file__).resolve().parents[1]
image_path=root/'assets/qr-code-original.png'
original=image_path.read_bytes()
img=Image.open(image_path).convert('L')
assert img.size==(2048,2048),img.size
pix=img.load()
# The original QR is 29x29 light modules on black; the raster has outer
# padding which is not part of the symbol. Identify the true light bounds.
coords=[(x,y) for y in range(img.height) for x in range(img.width) if pix[x,y]>127]
x0=min(x for x,y in coords); x1=max(x for x,y in coords)+1
y0=min(y for x,y in coords); y1=max(y for x,y in coords)+1
assert (x0,y0,x1,y1)==(269,269,1779,1779)
size=29
matrix=[]
for r in range(size):
 row=[]
 for c in range(size):
  x=round(x0+(c+.5)*(x1-x0)/size)
  y=round(y0+(r+.5)*(y1-y0)/size)
  row.append(int(pix[x,y]>127))
 matrix.append(row)
# Check that all samples are on stable flat areas of the source raster.
for r in range(size):
 for c in range(size):
  x=round(x0+(c+.5)*(x1-x0)/size)
  y=round(y0+(r+.5)*(y1-y0)/size)
  assert all(int(pix[x+dx,y+dy]>127)==matrix[r][c] for dx,dy in [(0,0),(-10,0),(10,0),(0,-10),(0,10)])
assert sum(sum(row) for row in matrix)>300
head=['/* Exact WHITE modules from the USER-ATTACHED qr-code.png.',
      ' * Original asset: assets/qr-code-original.png',
      ' * No QR regeneration, no module changes, no URL-based substitutes.',
      ' * Source file SHA-256: '+hashlib.sha256(original).hexdigest(),
      ' */', '#ifndef RUPTURE_BUG_QR_H','#define RUPTURE_BUG_QR_H','#include <stdint.h>',
      'static const uint8_t ru_bug_qr_white[29][4]={']
for row in matrix:
 packed=[]
 for i in range(0,29,8):
  value=0
  for j in range(8):
   if i+j<len(row) and row[i+j]:value |= 0x80>>j
  packed.append('0x%02x'%value)
 head.append(' {'+','.join(packed)+'},')
head+=['};','#endif']
header=root/'vendor/etch-v1.88-source/firmware/fm1-native/rupture_bug_qr.h'
header.write_text('\n'.join(head)+'\n')
print('Exact 29x29 user QR extracted; white modules',sum(map(sum,matrix)))
print('Original SHA-256:',hashlib.sha256(original).hexdigest())
