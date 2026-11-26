#!/usr/bin/env python3
"""The PCM delta is only one guarded post-gain optimization in CORE2."""
from pathlib import Path
import hashlib

root=Path(__file__).resolve().parents[1]
native=root/'vendor/etch-v1.88-source/firmware/fm1-native'
experimental=(native/'audio_core_core2.c').read_text()
stable=(native/'audio_core.c').read_bytes()
before='''  l=clamp15((l*(int32_t)v->post_gain)/32768);
  r=clamp15((r*(int32_t)v->post_gain)/32768);
'''
after='''  /* PCM from CPU1 and the preview mixer is already signed 16-bit. At
   * Q15 gain <= 32768 the product/division cannot leave that range,
   * so two saturation checks do no work. Retain the original exact
   * clamp path for malformed/out-of-range gains (up to UINT16_MAX). */
  const int32_t post=(int32_t)v->post_gain;
  if(post<=32768){
   l=(l*post)/32768;
   r=(r*post)/32768;
  }else{
   l=clamp15((l*post)/32768);
   r=clamp15((r*post)/32768);
  }
'''
sha=lambda x:hashlib.sha256(x).hexdigest()
assert experimental.count(after)==1
assert sha(experimental.replace(after,before,1).encode())=='f77eab6c8c6200c27be25c51f90a44d04c06fda533d6e3aa46999c3e3d1594f4'
assert sha(stable)=='0e5588d24d41f69be5e3065acf4218795a4b55916f448a1b595d984fde7eb235'
assert 'if(v->click)' in experimental
assert 'ru_cpu1_fx_collect();' in experimental
print('PASS V2.0.58: only CORE2 post-gain clamps skipped for Q15, high gains protected')
