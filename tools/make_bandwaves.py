#!/usr/bin/env python3
"""Generate fixed, bandlimited 256-phase oscillator tables offline; no runtime math."""
from pathlib import Path
from math import sin,pi
root=Path(__file__).resolve().parents[1]
out=root/'vendor/etch-v1.88-source/firmware/fm1-native/rupture_bandwaves.h'
lines=['/* SPDX-License-Identifier: GPL-3.0-only', ' * RUPTURE V1.45: offline additive Fourier tables; fixed 256-phase, 7 mip bands.', ' * Audio callback uses only 32-bit phase, lookup, integer interpolation.', ' * No runtime trigonometry, division, allocation, or table synthesis.', ' */','#ifndef RUPTURE_BANDWAVES_H','#define RUPTURE_BANDWAVES_H','#include <stdint.h>','#define RU_WAVE_SQUARE 3u','#define RU_WAVE_SAW 4u', '#define RU_WAVE_LEVELS 7u']
for name in ('square','saw'):
    vals=[]
    for limit in (1,2,4,8,16,32,64):
        buffer=[]
        for idx in range(256):
            phase=2*pi*idx/256
            if name=='saw':
                # rising saw (zero crossing at 0): spectrum (-1)^(n+1)/n
                v=sum(((-1)**(n+1))*sin(n*phase)/n for n in range(1,limit+1))*2/pi
            else:
                v=sum(sin(n*phase)/n for n in range(1,limit+1,2))*4/pi
            buffer.append(v)
        peak=max(abs(x) for x in buffer)
        # preserve loudness between band transitions with constant peak, no time-varying gain
        vals.append([int(round(x*29000/peak)) for x in buffer])
    lines.append('static const int16_t ru_'+name+'_waves[RU_WAVE_LEVELS][256]={')
    for i,row in enumerate(vals):
        lines.append(' { /* up to %d harmonics */'% (1<<i))
        for k in range(0,256,16): lines.append('  '+','.join(str(x) for x in row[k:k+16])+(',' if k+16<256 else ''))
        lines.append(' },' if i<6 else ' }')
    lines.append('};')
lines += [
 'static inline uint8_t ru_wave_band(uint32_t increment){',
 ' if(increment>0x7fffffffu)return 0u;',
 ' if(increment>0x3fffffffu)return 0u;',
 ' if(increment>0x1fffffffu)return 1u;',
 ' if(increment>0x0fffffffu)return 2u;',
 ' if(increment>0x07ffffffu)return 3u;',
 ' if(increment>0x03ffffffu)return 4u;',
 ' if(increment>0x01ffffffu)return 5u;',
 ' return 6u;',
 '}',
 'static inline int32_t ru_bandwave_level_read(uint8_t wave,uint8_t level,uint32_t phase){',
 ' uint8_t index=(uint8_t)(phase>>24);',
 ' uint32_t fraction=(phase>>16)&255u;',
 ' const int16_t *t=(wave==RU_WAVE_SQUARE?ru_square_waves[level]:ru_saw_waves[level]);',
 ' int32_t current=t[index];',
 ' return current+(((int32_t)t[(uint8_t)(index+1u)]-current)*(int32_t)fraction>>8);',
 '}',
 'static inline int32_t ru_bandwave_read(uint8_t wave,uint32_t phase,uint32_t increment){',
 ' uint8_t level=ru_wave_band(increment);',
 ' int32_t now=ru_bandwave_level_read(wave,level,phase);',
 ' if(level){',
 '  /* Crossfade only BELOW the harmonic Nyquist threshold, over its lowest',
 '   * 1/8 frequency band. No alias-prone harmonics above Nyquist. */',
 '  uint32_t threshold=0x7fffffffu>>level;',
 '  uint32_t fade_range=threshold>>3;',
 '  if(increment>threshold-fade_range && increment<=threshold){',
 '   uint32_t mix=(threshold-increment)>>(20u-level);',
 '   if(mix>256u)mix=256u;',
 '   int32_t previous=ru_bandwave_level_read(wave,(uint8_t)(level-1u),phase);',
 '   now=((now*(int32_t)mix)+(previous*(int32_t)(256u-mix)))>>8;',
 '  }',
 ' }',
 ' return now;',
 '}',
 '#endif'
]
out.write_text('\n'.join(lines)+'\n')
print(out,out.stat().st_size)
