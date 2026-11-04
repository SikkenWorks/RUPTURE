#!/usr/bin/env python3
"""Generate the original, offline antialiased 5.8 ms RUPTURE metronome click."""
import math
from pathlib import Path
root=Path(__file__).resolve().parents[1]
f=root/'vendor/etch-v1.88-source/firmware/fm1-native/rupture_metronome_wave.h'
rate=44100
samples=[]
for i in range(256):
 t=i/rate
 # Smooth attack and exponential decay, a clear 1760 Hz sine plus slight
 # 2nd harmonic. High harmonics taper to silence, not a discontinuous pulse.
 attack=min(1.,i/16)
 decay=math.exp(-t*560)
 tail=min(1.,(256-i)/20)
 val=(math.sin(2*math.pi*1760*t)+0.19*math.sin(2*math.pi*3520*t))
 samples.append(round(3800*attack*decay*tail*val))
# no dynamically calculated waves in the ISR
lines=['/* V1.72 original bandlimited 44.1kHz PCM metronome click. Generated offline.',
       ' * No runtime oscillator, trig, division, heap, effect or synth voice. */',
       '#ifndef RUPTURE_METRONOME_WAVE_H','#define RUPTURE_METRONOME_WAVE_H',
       '#include <stdint.h>','static const int16_t ru_metro_pcm[256]={']
for k in range(0,256,16):lines.append(' '+','.join(str(x) for x in samples[k:k+16])+',')
lines+=['};','#endif']
f.write_text('\n'.join(lines)+'\n')
print('Generated',f,'peak',max(abs(x) for x in samples),'samples',len(samples))
