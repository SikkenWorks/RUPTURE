#!/usr/bin/env python3
"""Export exact pixel frames using RUPTURE firmware's rupture_ui.c, not a redraw."""
import subprocess, pathlib
from PIL import Image
root=pathlib.Path(__file__).resolve().parents[1]
n=root/'vendor/etch-v1.88-source/firmware/fm1-native'
exe=root/'tests/.rupture_host_export'
subprocess.run(['gcc','-O2','-Wno-unused-function','-I',str(n),str(n/'rupture_host_export.c'),'-o',str(exe)],check=True)
try:
 names=['PLAY', 'FILTER', 'ENVELOPE', 'LFO', 'FX', 'REVERB', 'DELAY', 'CHORUS', 'OVERDRIVE', 'SEQUENCER', 'SEQ SLOTS', 'ARPEGGIATOR', 'SCALE GUIDE', 'KEYLOCK', 'PROJECTS', 'GLO', 'SETTINGS', 'HELP', 'UPDATE', 'FOREIGN DATA', 'FORMAT CONFIRM', 'SAFE MODE', 'NOTE MEMORY', 'MIDI', 'LOADING', 'MENU', 'CHANNEL EQ', 'SOUND SHAPE', 'NAME EDITOR', 'SUB OSC 1', 'SUB OSC 2', 'COMPRESSOR', 'WAVEFOLDER', 'TRANSIENT SHAPER', 'TILT EQ', 'OCTAVE FUZZ', 'SANDPAPER', 'AUDIO DIAGNOSTICS','STORAGE','STORAGE PROJECTS','STORAGE ACTIONS','STORAGE SEQUENCES','STORAGE SEQ ACTIONS','WIPE ALL','OTHER','ABOUT','REPORT A BUG','SCREENSAVER OPTIONS','AUTO LOAD','WELCOME','SYSTEM INFO']
 for i,name in enumerate(names):
  if i in (32,33,49):continue # V1.22: removed WAVEFOLDER and TRANSIENT pages
  output=root/'ui/screens'/(name.lower().replace(' ','-')+'.png')
  temp=output.with_suffix('.pgm')
  subprocess.run([str(exe),str(i),str(temp)],check=True)
  Image.open(temp).save(output,optimize=True)
  temp.unlink()
 for pg,mode,label in [(0,'saw','play-saw'),(27,'saw','sound-shape-saw'),
                       (29,'square','sub-osc-1-square'),(29,'saw','sub-osc-1-saw'),
                       (30,'square','sub-osc-2-square'),(30,'saw','sub-osc-2-saw')]:
  output=root/'ui/screens'/(label+'.png');temp=output.with_suffix('.pgm')
  subprocess.run([str(exe),str(pg),str(temp),mode],check=True)
  Image.open(temp).save(output,optimize=True);temp.unlink()
 for title in ('eq2-gain','eq2-freq','eq2-width'):
  output=root/'ui/screens'/(title+'.png');temp=output.with_suffix('.pgm')
  subprocess.run([str(exe),title,str(temp)],check=True)
  Image.open(temp).save(output,optimize=True);temp.unlink()
 for scene in ('audio-fault',):
  output=root/'ui/screens'/(scene+'.png');temp=output.with_suffix('.pgm')
  subprocess.run([str(exe),scene,str(temp)],check=True)
  Image.open(temp).save(output,optimize=True);temp.unlink()
 for scene in ('storage-save','storage-load'):
  output=root/'ui/screens'/(scene+'.png');temp=output.with_suffix('.pgm')
  subprocess.run([str(exe),scene,str(temp)],check=True)
  Image.open(temp).save(output,optimize=True);temp.unlink()
 for frame in (0,8,20,32,48,60):
  pgm=root/'ui/screens'/f'screensaver-{frame:02d}.pgm'
  subprocess.run([str(exe),'saver',str(pgm),str(frame)],check=True)
  Image.open(pgm).save(pgm.with_suffix('.png'),optimize=True)
  pgm.unlink()
 for old in ('wavefolder.png','transient-shaper.png'):
  (root/'ui/screens'/old).unlink(missing_ok=True)
 frame_dir=root/'ui/loading-frames'
 frame_dir.mkdir(exist_ok=True)
 for step in range(1,25):
  pgm=frame_dir/(f'frame-{step:02d}.pgm')
  subprocess.run([str(exe),'24',str(pgm),str(step)],check=True)
  Image.open(pgm).save(pgm.with_suffix('.png'),optimize=True)
  pgm.unlink()
finally:exe.unlink(missing_ok=True)
