#!/usr/bin/env python3
"""Verify all OCTAVE FUZZ reciprocal-table indices and approximation error."""
from pathlib import Path
import re
s=(Path(__file__).resolve().parents[1]/'vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c').read_text()
m=re.search(r'rupture_fuzz_recip_q24\[518\]=\{(.*?)\};',s,re.S)
assert m, 'V1.42 fixed reciprocal table missing'
array=[int(x) for x in re.findall(r'\d+',m.group(1))]
assert len(array)==518
maxerr=0
for wet_ref in range(256,33024):
 i=wet_ref>>6;f=wet_ref&63
 recip=array[i]+int((array[i+1]-array[i])*f/64)
 for dry_ref in (256,wet_ref//2,wet_ref-1):
  if dry_ref>=wet_ref:continue
  actual=min(32767,max(4096,(dry_ref*recip)>>9))
  expected=min(32767,max(4096,(dry_ref*32767)//wet_ref))
  maxerr=max(maxerr,abs(actual-expected))
assert maxerr<=400,(maxerr, 'ratio target differs too much')
block=s[s.index('static int16_t rupture_fuzz_process'):s.index('/* SANDPAPER:')]
assert '/wet_ref' not in block
assert 'rupture_fuzz_ratio_q15' in block
print('PASS V1.42 fixed reciprocal: 32,768 denominator values, max ratio target error',maxerr,'/ 32767')
