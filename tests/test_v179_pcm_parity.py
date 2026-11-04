#!/usr/bin/env python3
"""Test exact V1.78 vs V1.79 complete FX PCM against committed source reference."""
import hashlib,subprocess,tempfile
from pathlib import Path
r=Path(__file__).resolve().parents[1]
n=r/'vendor/etch-v1.88-source/firmware/fm1-native'
with tempfile.TemporaryDirectory() as td:
 out=[]
 for kind,src in [('V1.78',r/'reference/native_fx-V1.78.c'),('V1.79',n/'native_fx.c')]:
  exe=Path(td)/(kind+'.exe')
  subprocess.run(['gcc','-std=c11','-O2','-DRUPTURE_BUILD','-Wno-unused-function','-I'+str(n),str(r/'tests/test_v179_fx_pcm.c'),str(src),str(n/'rupture_eq2.c'),'-lm','-o',str(exe)],check=True)
  audio=subprocess.check_output([str(exe)])
  out.append(hashlib.sha256(audio).hexdigest())
 assert out[0]==out[1],f'PCM regression: {out}'
 assert out[0]=='93b98d02f77d868766cec5630a7b624133feebb7de0e2f37b55f6b54f30cea23'
 print('PASS V1.79 exact 780000-byte 3-engine all-effects PCM parity:',out[0])
