#!/usr/bin/env python3
"""Pixel parity for all twenty original 1-bit sources vs native renderer."""
import pathlib,subprocess,tempfile
from PIL import Image,ImageChops
r=pathlib.Path(__file__).resolve().parents[1]
n=r/'vendor/etch-v1.88-source/firmware/fm1-native'
files=sorted((r/'assets/dam-v3-frames/frames').glob('*.png'))
assert len(files)==20
with tempfile.TemporaryDirectory() as d:
 exe=pathlib.Path(d)/'export'
 subprocess.run(['gcc','-O2','-Wno-unused-function','-I',str(n),str(n/'rupture_host_export.c'),'-o',str(exe)],check=True)
 bg=None
 for i,f in enumerate(files):
  out=pathlib.Path(d)/f'out-{i}.pgm'
  subprocess.run([str(exe),'0',str(out),f'dam{i}'],check=True)
  screen=Image.open(out).convert('L')
  original=Image.open(f).convert('L')
  assert original.size==(224,30)
  # V1.58+ deliberately shifts each moving sprite by one pixel to
  # produce the FM1 shake effect.  Compare the exact original PNG after
  # applying that established UI-only offset, not the obsolete static crop.
  xoff=(-1 if i%3==1 else 1 if i%3==2 else 0) if 0<i<19 else 0
  yoff=(-1 if i==11 else 1 if i==12 else 0)
  expected=Image.new('L',(240,30),0)
  expected.paste(original,(8+xoff,yoff))
  actual=screen.crop((0,50,240,80))
  assert ImageChops.difference(actual,expected).getbbox() is None,(i,f)
  # Only the 30px-high panel may differ, independent of the frame origin.
  p=screen.load()
  for y in range(50,80):
   for x in range(240):p[x,y]=0
  if bg is None:bg=screen.copy()
  else:assert ImageChops.difference(bg,screen).getbbox() is None,i
 print('20/20 original PNG sprites rendered pixel-exact with established V1.58 shake offsets; outside-panel pixels unchanged')
