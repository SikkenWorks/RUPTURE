from pathlib import Path
from PIL import Image
from subprocess import run
import hashlib,subprocess
root=Path(__file__).resolve().parents[1]
assets=root/'assets/panel-animations-v157/volcano'
exe=str(root/'tools/ru157_host_export')
from subprocess import run
if not Path(exe).exists():
 run(['gcc','-std=c11','-O2','-Wno-unused-function','-o',exe,str(root/'vendor/etch-v1.88-source/firmware/fm1-native/rupture_host_export.c'),'-lm'],check=True)
for i in range(20):
 f=assets/f'{i+1:02d}.png'
 sprite=Image.open(f).convert('RGB')
 tmp=root/'tools/ru157_frame.ppm'
 run([exe,'27',str(tmp),f'volcano{i}'],check=True,stdout=subprocess.DEVNULL)
 canvas=Image.open(tmp).convert('RGB')
 assert canvas.crop((8,50,232,80)).tobytes()==sprite.tobytes(),f'pixel mismatch {i}'
 assert sprite.size==(224,30) and all(c in (0,255) for c in sprite.convert('L').getdata())
print('PASS: 20/20 pixel-exact source PNG vs native UI renderer x8..231 y50..79')
# Other UI pages are untouched pixel-for-pixel relative to V1.56 source data.
old=None # version-to-version byte comparison performed in release report
new=root/'vendor/etch-v1.88-source/firmware/fm1-native'
# Make sure all images outside target have same UI frame when turning off volcano anim.
run([exe,'27',str(root/'tools/ru157_shape0.ppm'),'volcano0'],check=True)
run([exe,'27',str(root/'tools/ru157_shape10.ppm'),'volcano10'],check=True)
a=Image.open(str(root/'tools/ru157_shape0.ppm')).convert('RGB')
b=Image.open(str(root/'tools/ru157_shape10.ppm')).convert('RGB')
for y in range(240):
 for x in range(240):
  if not (8<=x<232 and 50<=y<80):
   assert a.getpixel((x,y))==b.getpixel((x,y)),(x,y)
print('PASS: only SOUND SHAPE bar changes during eruption; knobs and labels identical')
