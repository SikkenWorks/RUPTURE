#!/usr/bin/env python3
"""Native 240x240 BEND previews: 5 K3 curves x 8 ROM frames x CRT ON/OFF."""
from pathlib import Path
from PIL import Image, ImageDraw
import subprocess, tempfile
R=Path(__file__).resolve().parents[1]
N=R/'vendor/etch-v1.88-source/firmware/fm1-native'
OUT=R/'assets/bend-v188-native-ui'
OUT.mkdir(parents=True,exist_ok=True)
BIN=OUT/'exporter-host'
subprocess.run(['cc','-std=c11','-O2','-DRUPTURE_BUILD=1',str(N/'rupture_host_export.c'),'-lm','-o',str(BIN)],check=True,cwd=N)
curves=['LINEAR-EVEREST','SOFT-MONOLITH','HARD-PIRATE-SHIP','S-CURVE-CAMERA','EXP-KRAKEN']
for ci,name in enumerate(curves):
 for i in range(8):
  for crt in [False,True]:
   mode=f'bend{ci}-frame{i}-shake2'+('-crt-on' if crt else '')
   path=OUT/f'{name}-FRAME-{i:02}-CRT-{"ON" if crt else "OFF"}.png'
   with tempfile.TemporaryDirectory() as tmp:
    raw=Path(tmp)/'frame.ppm'
    subprocess.run([str(BIN),'32',str(raw),mode],check=True,stdout=subprocess.DEVNULL)
    im=Image.open(raw).convert('RGB')
    assert im.size==(240,240)
    im.save(path)
# Overview: frame 7, both CRT states, with labels and 240x240 original previews
sheet=Image.new('RGB',(240*5,240*2),(0,0,0))
for ci,name in enumerate(curves):
 for j,crt in enumerate([False,True]):
  im=Image.open(OUT/f'{name}-FRAME-07-CRT-{"ON" if crt else "OFF"}.png')
  sheet.paste(im,(ci*240,j*240))
sheet.save(OUT/'BEND-V1.88-FIVE-CURVES-CRT-ON-OFF.png')
# Include a 2x pixel-scale version (no interpolation)
sheet.resize((240*5*2,240*2*2),Image.Resampling.NEAREST).save(OUT/'BEND-V1.88-FIVE-CURVES-CRT-ON-OFF-2X.png')
# Test shake: the close-up must be phase independent; four other sets must jitter.
for ci in range(5):
 images=[]
 for phase in [0,2]:
  with tempfile.TemporaryDirectory() as tmp:
   raw=Path(tmp)/'render.ppm'
   subprocess.run([str(BIN),'32',str(raw),f'bend{ci}-frame7-shake{phase}'],check=True,stdout=subprocess.DEVNULL)
   images.append(raw.read_bytes())
 if ci==3: assert images[0]==images[1], 'Close-up must not shake'
 else: assert images[0]!=images[1], f'Curve {ci} must shake'
print('PASS: 5 curves x 8 frames x 2 CRT settings = 80 native 240x240 render previews')
print('PASS: all four eruption sequences shake; S-CURVE close-up stays completely stable')
BIN.unlink()
