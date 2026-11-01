#!/usr/bin/env python3
"""Test exact user-uploaded QR against packed ROM and rendered FM1 page."""
from pathlib import Path
from PIL import Image
import hashlib, re, subprocess, tempfile, numpy as np, cv2
root=Path(__file__).resolve().parents[1]
source=root/'assets/qr-code-original.png'
assert hashlib.sha256(source.read_bytes()).hexdigest()=='10c58f4684cea5dcbe9597e5569657f5c63fa74e33a4102a24b40342e16ac9f2'
orig=np.asarray(Image.open(source).convert('L')) > 127
ys,xs=np.where(orig)
x0,x1,y0,y1=xs.min(),xs.max()+1,ys.min(),ys.max()+1
assert (x0,x1,y0,y1)==(269,1779,269,1779)
real=np.array([[orig[round(y0+(r+.5)*(y1-y0)/29),round(x0+(c+.5)*(x1-x0)/29)] for c in range(29)] for r in range(29)])
hdr=(root/'vendor/etch-v1.88-source/firmware/fm1-native/rupture_bug_qr.h').read_text()
packed=re.findall(r'\{(0x[0-9a-fA-F, x]+)\}',hdr)
assert len(packed)==29
embedded=np.array([[bool(int(line.split(',')[col//8].strip(),16) & (0x80>>(col%8))) for col in range(29)] for line in packed])
assert (real==embedded).all(), 'User supplied QR modules differ from embedded firmware!'
png=root/'ui/screens/report-a-bug.png'
actual=np.array(Image.open(png).convert('L'))>127
assert actual.shape==(240,240)
for row in range(29):
 for col in range(29):
  block=actual[52+5*row:52+5*row+5,47+5*col:47+5*col+5]
  assert block.shape==(5,5) and (block==real[row,col]).all(), ('pixel mismatch',row,col)
# The real original QR is inverted visually; the QR scanner parses it after
# polarity correction, which must not modify the user-authored bitmap.
reader=cv2.QRCodeDetector()
original_decoded=reader.detectAndDecode(255-np.asarray(Image.open(source).convert('L')))[0]
assert original_decoded=='https://github.com/SikkenWorks/RUPTURE/issues', original_decoded
print('PASS: original SHA; 841/841 QR modules match source; 21025/21025 display pixels identical; original destination preserved')
