from PIL import Image,ImageChops
from pathlib import Path
import subprocess,tempfile
root=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory() as d:
 d=Path(d)
 n=root/'vendor/etch-v1.88-source/firmware/fm1-native'
 exe=d/'export'
 subprocess.run(['gcc','-O2','-Wno-unused-function','-I',str(n),str(n/'rupture_host_export.c'),'-o',str(exe)],check=True)
 for page,kind in [(3,'lfo'),(2,'env')]:
  base=None
  for i in range(20):
   result=d/f'{kind}{i}.pgm'
   subprocess.run([str(exe),str(page),str(result),f'{kind}{i}'],check=True)
   im=Image.open(result).convert('L')
   sprite=Image.open(root/'assets/panel-animations-v156'/kind/f'{i+1:02d}.png').convert('L')
   actual=im.crop((8,50,232,80))
   assert ImageChops.difference(actual,sprite).getbbox() is None, (kind,i)
   # check baseline unaffected pixels outside the sprite area
   mask=im.copy();p=mask.load()
   for y in range(50,80):
    for x in range(8,232):p[x,y]=0
   if base is None:base=mask.copy()
   assert ImageChops.difference(base,mask).getbbox() is None,(kind,i)
  print(f'{kind.upper()}: 20/20 PNGs bitwise identical to the native renderer; rest of UI unchanged')
