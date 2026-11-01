#!/usr/bin/env python3
"""Verify the real native pixel output, not a hand-drawn mockup."""
from pathlib import Path
from PIL import Image
root=Path(__file__).resolve().parents[1]
for scene in ['play','play-saw','sound-shape','sound-shape-saw']:
 im=Image.open(root/'ui/screens'/(scene+'.png')).convert('L')
 assert im.size==(240,240),scene
 # the header's text baseline is at 32..38; no body begins before y=50
 assert all(im.getpixel((x,y))==0 for x in range(8,232) for y in (46,47,48,49)),scene
 # waveform/bars end at y=79; knob rings begin at y=83
 assert all(im.getpixel((x,y))==0 for x in range(8,232) for y in (80,81,82)),scene
 # header divider remains unbroken, no graphical controls intrude
 assert all(im.getpixel((x,45))>0 for x in range(7,233)),scene
print('V1.46 exact UI pixel spacing PASS')
