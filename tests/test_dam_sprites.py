#!/usr/bin/env python3
"""Pixel parity for all twelve original 1-bit sources vs native renderer."""
import pathlib,subprocess,tempfile
from PIL import Image,ImageChops
r=pathlib.Path(__file__).resolve().parents[1]
n=r/'vendor/etch-v1.88-source/firmware/fm1-native'
files=sorted((r/'assets/dam-frames').glob('*.png'))
assert len(files)==12
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
  actual=screen.crop((8,50,232,80))
  assert ImageChops.difference(actual,original).getbbox() is None,(i,f)
  # EVERYTHING outside bar must be unchanged when the animation changes
  p=screen.load()
  for y in range(50,80):
   for x in range(8,232):p[x,y]=0
  if bg is None:bg=screen.copy()
  else:assert ImageChops.difference(bg,screen).getbbox() is None,i
 print('12/12 pixel-exact original PNGs, native UI x=8..231 y=50..79; all out-of-bar pixels identical')
