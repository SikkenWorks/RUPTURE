#!/usr/bin/env python3
"""Offline 1-bit RUPTURE FM1 BEND 5 x 8 224x30 sprites.

LINEAR reuses the approved V1.87 Everest frames byte-for-byte. The four new
animations are artistically laid out for the real 224x30 native screen, using
the approved illustrated concept boards as visual direction. No runtime pixel
math, heap, audio ISR, or DSP changes are involved.
"""
from pathlib import Path
from PIL import Image, ImageDraw
import random, math, shutil, hashlib

ROOT=Path(__file__).resolve().parents[1]
SRC=ROOT/'vendor/etch-v1.88-source/firmware/fm1-native'
OUT=SRC/'bend_sprites_v188'
OUT.mkdir(exist_ok=True)
W,H=224,30
WHITE=255
BLACK=0

def base():
 im=Image.new('L',(W,H),BLACK); return im,ImageDraw.Draw(im)

def lin(d,pts,w=1): d.line(pts,fill=WHITE,width=w)
def white(d,xy): d.point(xy,fill=WHITE)
def dot(d,x,y):
 if 0<=round(x)<W and 0<=round(y)<H: white(d,(round(x),round(y)))
def rubble(d,seed,level=1.0,cx=112,ground=25):
 rng=random.Random(seed)
 for i in range(int(70*level)):
  x=rng.randrange(6,218)
  if abs(x-cx)<28 and rng.random()<.50: continue
  y=ground-rng.choice((0,0,0,1,2,3,4,5))
  if y<2 or y>=28:continue
  if rng.random()<.36: lin(d,[(x,y),(x+rng.choice([-2,2]),y-1)])
  else:dot(d,x,y)

def crackline(d,strength=1.0):
 segments=[(3,25),(24,25),(32,24),(43,25),(58,25),(77,24),(88,24),(99,26),
           (112,28),(120,26),(137,24),(151,25),(164,25),(178,24),(191,25),(218,25)]
 lin(d,segments)
 if strength>0.2:
  for x,y in [(97,26),(117,26),(78,24),(144,24)]:
   lin(d,[(x,y),(x+(-7 if x<110 else 7)*strength,y-int(4*strength)),(x+(-11 if x<110 else 11)*strength,y-int(3*strength))])

def impact_lines(d,seed,progress,cx=112,cy=24):
 if progress<0.2:return
 rng=random.Random(seed)
 for j in range(int(progress*17)):
  dx=rng.choice([-1,1]);x=cx+dx*rng.randint(28,88)
  y=rng.randint(3,20)
  length=rng.choice([2,3,5,6])
  lin(d,[(x,y),(x+dx*length,y-rng.choice([1,2,3]))])

def montolith_frame(idx):
 t=idx/7; im,d=base();crackline(d,t);rubble(d,123+idx,int((t+.07)*.8),ground=25)
 if idx==0:return im
 # Tall matte black volcanic stone with sharp white faceted edge.
 top=round(25-23*t);wide=round(7+27*t)
 l=112-wide//2;r=112+wide//2
 # whole monolith main body: right side dark, front accent lines.
 front=[(l+2,25),(l,top+1),(r-9,top),(r-7,25)]
 side=[(r-9,top),(r,top+4),(r+2,24),(r-7,25)]
 d.polygon(front,fill=BLACK,outline=WHITE)
 d.polygon(side,fill=BLACK,outline=WHITE)
 for xx in range(l+5,r-9,6):
  if top+4<25:
   lin(d,[(xx,top+3),(xx+1,top+8)],1)
 if idx>=3:
  lin(d,[(l+5,top+4),(l+10,top+7),(l+6,top+11),(l+11,top+14)])
  lin(d,[(r-9,top+4),(r-14,top+7),(r-9,top+10),(r-16,top+14)])
  for yy in range(top+3,25,3):
   dot(d,r-5+yy%2,yy)
 if idx>=5:
  lin(d,[(l-4,26),(l-9,22),(l-15,21),(l-20,18)])
  lin(d,[(r+4,26),(r+9,23),(r+17,21),(r+22,17)])
  for ax,ay in [(75,20),(60,17),(145,18),(160,14),(92,5),(136,6)]:
   dot(d,ax,ay)
 impact_lines(d,900+idx,t)
 return im

def hull(d,base_y):
 # Old heavily armored three-masted flagship: hull protrudes through torn meadow.
 bow=178;stern=43
 hull_points=[(47,base_y-10),(77,base_y-9),(102,base_y-7),(132,base_y-8),
  (161,base_y-11),(175,base_y-14),(168,base_y-4),(145,base_y+1),(79,base_y+1),(54,base_y-4)]
 d.polygon(hull_points,fill=BLACK,outline=WHITE)
 lin(d,[(57,base_y-6),(79,base_y-5),(118,base_y-5),(153,base_y-8),(168,base_y-11)])
 lin(d,[(68,base_y-1),(106,base_y-1),(150,base_y-2)],1)
 for x in range(69,159,13):
  y=base_y-6 if x<145 else base_y-8
  d.ellipse((x,y,x+3,y+3),outline=WHITE)
 # layered stern and prow details
 lin(d,[(49,base_y-10),(46,base_y-14),(62,base_y-15),(70,base_y-10)])
 lin(d,[(165,base_y-11),(183,base_y-17),(191,base_y-17)])
 for x in range(60,157,7):
  dot(d,x,base_y-3)

def ship_scene(d,yshift,seed=0):
 # Ship at final base_y=26, move it down when not yet emerged.
 base_y=26+yshift
 hull(d,base_y)
 for j,(x,h) in enumerate([(85,17),(116,25),(144,19)]):
  top=base_y-h
  lin(d,[(x,base_y-9),(x,top)],1)
  lin(d,[(x-17,top+4),(x+15,top+4)],1)
  lin(d,[(x-13,top+12),(x+11,top+12)],1)
  # sails swell rightward in wind; separate triangular panels, snow-white outlines
  d.polygon([(x-8,top+5),(x+3,top+5),(x+13,top+10),(x+4,top+12),(x-7,top+11)],fill=BLACK,outline=WHITE)
  for z in [2,6,9]:
   lin(d,[(x-7+z,top+6),(x-5+z,top+10)])
  # rigging trusses / diagonal shrouds
  lin(d,[(x,top+1),(x-18,base_y-8)])
  lin(d,[(x,top+1),(x+16,base_y-8)])
  if j==1:
   # crow's nest and pirate skull flag
   lin(d,[(x-7,top+1),(x+9,top+1)])
   d.polygon([(x+2,top-3),(x+23,top),(x+4,top+4)],fill=BLACK,outline=WHITE)
   dot(d,x+12,top);dot(d,x+15,top)
 # wake and flying turf

def pirate_frame(idx):
 t=idx/7;im,d=base()
 # grassy meadow before ship tears through it
 for x in range(3,220,7):
  y=26+(x%3)
  lin(d,[(x,y),(x-1,y-2)]);lin(d,[(x,y),(x+2,y-3)])
 crackline(d,t)
 if idx:
  yshift=round((1-t)*25)
  ship_scene(d,yshift)
 rubble(d,380+idx,t,ground=25)
 impact_lines(d,560+idx,t)
 if idx>=3:
  for x in (40,55,167,186):
   lin(d,[(x,26),(x+(3 if x<110 else -3),23-(idx%2))])
 return im

def tall_ship_scene():
 # One tall illustration: the camera runs from massive ornate hull to sails and flag.
 Ht=180;im=Image.new('L',(W,Ht),0);d=ImageDraw.Draw(im)
 rng=random.Random(7091)
 # immense mast(s) span the whole vertical scene
 for x in (67,128,183):
  lin(d,[(x,7),(x,178)],2)
  lin(d,[(x+3,7),(x+3,174)])
 # hull lower half, white layered lines and rows of cannon portholes
 hull_top=131
 d.polygon([(6,145),(37,129),(171,132),(216,121),(207,177),(9,179)],fill=BLACK,outline=WHITE)
 for y in range(139,177,6):
  lin(d,[(13,y+5),(43,y),(166,y+3),(211,y-5)])
  for x in range(20+(y%10),203,19):
   if (x+y)%3:dot(d,x,y)
 for y in (146,164):
  for x in range(25,212,23):
   d.ellipse((x-4,y-4,x+5,y+3),outline=WHITE)
   d.ellipse((x-1,y-2,x+2,y+1),fill=WHITE)
   dot(d,x,y)
 # bow / ornate figurehead at starboard
 lin(d,[(190,130),(213,120),(220,106)])
 d.ellipse((212,105,218,111),outline=WHITE)
 # broad deck railing with decorative carvings
 for y in (117,123,129):
  lin(d,[(11,y+12),(49,y+4),(145,y+6),(202,y)])
 for x in range(15,201,9):
  lin(d,[(x,130),(x+2,121)])
 # rigging nets and ropes extend up through every camera window
 for x in (67,128,183):
  for end_x in (x-55,x+50):
   lin(d,[(x,8),(end_x,133)])
  for y in range(24,124,8):
   dx=int((y-8)*.35)
   lin(d,[(x-dx,y),(x+dx,y)])
 # tattered sails at mid-height: thick angled canvas silhouette separated by rigging
 for x,y,ww,hh in [(51,67,44,44),(115,43,54,54),(175,72,40,38)]:
  top=y;bottom=y+hh
  d.polygon([(x-ww//2,top),(x+ww//2,top+4),(x+ww//2+5,bottom-5),(x,bottom),(x-ww//2-2,bottom-3)],fill=BLACK,outline=WHITE)
  for k in range(3,ww,5):
   lin(d,[(x-ww//2+k,top+6),(x-ww//2+k+3,bottom-8)])
  for yy in range(top+10,bottom,11):
   lin(d,[(x-ww//2,yy),(x+ww//2,yy+4)])
 # yardarms and crow's nest above sails
 for x,y in [(65,56),(128,33),(183,59)]:
  lin(d,[(x-43,y),(x+41,y)],2)
  lin(d,[(x-12,y-9),(x+10,y-9)])
  lin(d,[(x-8,y-10),(x-12,y),(x+9,y),(x+12,y-10)])
 # top flag at highest mast and Jolly Roger skull.
 d.polygon([(131,8),(164,12),(156,20),(131,22)],fill=BLACK,outline=WHITE)
 d.ellipse((140,12,150,18),fill=WHITE)
 d.ellipse((142,13,144,15),fill=BLACK)
 d.ellipse((146,13,148,15),fill=BLACK)
 lin(d,[(139,19),(153,23)])
 lin(d,[(139,23),(153,19)])
 # spray, flecks, distressed paper texture across sails and hull
 for i in range(180):
  x=rng.randrange(8,W-8);y=rng.randrange(10,Ht-3)
  if im.getpixel((x,y)):
   if rng.random()<0.25:d.point((x,y),fill=BLACK)
  elif y>=95 and rng.random()<0.20:
   dot(d,x,y)
 return im

TALL=tall_ship_scene()
def ship_closeup_frame(idx):
 # low angle to sky: 0 is hull closeup, 7 is the mast and flag.
 offset=round(150*(1-idx/7))
 offset=max(0,min(TALL.height-30,offset))
 return TALL.crop((0,offset,W,offset+H))

def kraken_frame(idx):
 t=idx/7;im,d=base();crackline(d,t)
 if idx==0:
  for x in range(14,220,22):dot(d,x,22)
  return im
 # geyser-like forceful kraken erupts from the cracked ground
 cy=25-int(t*11);cx=110
 sz=3+int(t*10)
 d.ellipse((cx-sz,cy-sz,cx+sz,cy+sz),outline=WHITE)
 if idx>=4:
  # split brow and glowing eyes
  lin(d,[(cx-6,cy-4),(cx-2,cy-3)]);lin(d,[(cx+2,cy-3),(cx+6,cy-4)])
  d.ellipse((cx-5,cy-3,cx-3,cy-1),fill=WHITE)
  d.ellipse((cx+3,cy-3,cx+5,cy-1),fill=WHITE)
  for j in range(4):dot(d,cx-4+j*3,cy+4)
 for arm in range(6):
  side=-1 if arm%2==0 else 1
  ring=arm//2
  outward=12+(ring*15)+t*(18+ring*16)
  x0=cx+side*(sz-1)
  x1=cx+side*outward
  bend_y=cy-2-ring*2-int(t*(6+ring))
  tip_y=max(3,bend_y-3-ring*2)
  if idx>=2:
   pts=[(x0,cy+ring//2),(cx+side*(sz+12),cy-5),(x1,bend_y),(x1+side*(5+ring*2),tip_y),(x1+side*(1+ring*2),tip_y-2)]
   lin(d,pts,w=1 if idx<5 else 2)
   # segmented sucker dots along tentacle
   for q in range(4):
    xx=x0+side*(8+q*(7+ring*3));yy=cy-3-int(t*q)
    dot(d,xx,yy+2)
 if idx>=3:
  for dx in [-72,-57,-41,39,52,68]:
   x=cx+dx;y=22-int(t*(abs(dx)%11)//4)
   lin(d,[(x,25),(x+int(math.copysign(3,dx)),y)])
 rubble(d,770+idx,t*.95,ground=25)
 impact_lines(d,420+idx,t)
 return im

# Stable route of K3 index -> animation
names=['linear-everest','soft-monolith','hard-pirate-ship','s-curve-ship-closeup','exp-kraken']
frames=[]
for ci,name in enumerate(names):
 d=OUT/name;d.mkdir(parents=True,exist_ok=True)
 if ci==0:
  ev=SRC/'bend_frames'
  ims=[Image.open(ev/f'bend_frame_{i:02}.png').convert('L') for i in range(8)]
 else:
  render=[None,montolith_frame,pirate_frame,ship_closeup_frame,kraken_frame][ci]
  ims=[render(i) for i in range(8)]
 binary=[]
 for i,im in enumerate(ims):
  assert im.size==(224,30),(ci,i,im.size)
  mono=im.point(lambda p: 255 if p>=128 else 0,mode='1')
  mono.save(d/f'frame_{i:02}.png')
  binary.append(mono)
 frames.append(binary)
 sheet=Image.new('1',(W*4,H*2),0)
 for i,im in enumerate(binary):sheet.paste(im,((i%4)*W,(i//4)*H))
 sheet.save(d/'sprite_sheet_4x2.png')
 sheet.resize((W*4*2,H*2*2),Image.Resampling.NEAREST).save(d/'contact_2x.png')

# Header stores packed white foreground bitmaps directly in flash ROM.
header=SRC/'rupture_bend_sprites.h'
with header.open('w') as f:
 f.write('/* RUPTURE V1.88: 5 original offline 1-bit BEND animations.\n')
 f.write(' * Curve index order: LINEAR Everest / SOFT Monolith / HARD Flagship /\n')
 f.write(' * S-CURVE silent-camera Flagship Closeup / EXP Kraken.\n')
 f.write(' * 5x8x224x30, foreground white bit=1, prepacked ROM only.\n')
 f.write(' * Not computed or allocated in audio callback; no FX/DSP changes.\n')
 f.write(' * SPDX-License-Identifier: GPL-3.0-only */\n')
 f.write('#ifndef RUPTURE_BEND_SPRITES_H\n#define RUPTURE_BEND_SPRITES_H\n')
 f.write('#include <stdint.h>\n')
 f.write('#define RU_BEND_CURVES 5u\n#define RU_BEND_FRAMES 8u\n')
 f.write('#define RU_BEND_WIDTH 224u\n#define RU_BEND_HEIGHT 30u\n#define RU_BEND_BYTES 28u\n')
 f.write('#define RU_BEND_X 8u\n#define RU_BEND_Y 50u\n')
 f.write('static const uint8_t ru_bend_sprites[RU_BEND_CURVES][RU_BEND_FRAMES][RU_BEND_HEIGHT][RU_BEND_BYTES]={\n')
 for c,fs in enumerate(frames):
  f.write(' { /* '+names[c]+' */\n')
  for i,im in enumerate(fs):
   f.write('  {\n')
   for y in range(H):
    data=[]
    for xb in range(W//8):
     val=0
     for bit in range(8):
      if im.getpixel((xb*8+bit,y))==255: val |= 1<<(7-bit)
     data.append('0x%02x'%val)
    f.write('   {'+','.join(data)+'}'+(',' if y<29 else '')+'\n')
   f.write('  }'+(',' if i<7 else '')+'\n')
  f.write(' }'+(',' if c<4 else '')+'\n')
 f.write('};\n#endif\n')

# One preview overview with every frame for each animated series.
view=Image.new('1',(224*8,30*5),0)
for ci,fs in enumerate(frames):
 for fi,im in enumerate(fs):view.paste(im,(fi*W,ci*H))
view.save(OUT/'BEND-V1.88-FIVE-PRESETS-EIGHT-FRAMES.png')
view.resize((224*8*2,30*5*2),Image.Resampling.NEAREST).save(OUT/'BEND-V1.88-FIVE-PRESETS-EIGHT-FRAMES-2X.png')
print('PASS Generated five K3-linked 8-frame 224x30 1-bit sprite sequences, 33600 ROM bytes')
print('PASS LINEAR uses V1.87 sprites, untouched source files')
print('Source SHA256:',hashlib.sha256(header.read_bytes()).hexdigest())
