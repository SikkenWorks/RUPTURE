#!/usr/bin/env python3
"""Anchor historic V1.78/V1.89 PCM and the intentional V2.0.4 FX sound."""
import hashlib,subprocess,tempfile
from pathlib import Path
r=Path(__file__).resolve().parents[1]
n=r/'vendor/etch-v1.88-source/firmware/fm1-native'
with tempfile.TemporaryDirectory() as td:
 out=[]
 for kind,src in [('V1.78',r/'reference/native_fx-V1.78.c'),
                  ('V2.0.3_BASELINE',r/'reference/native_fx-V2.0.3.c'),
                  ('V2.0.4_FX',n/'native_fx.c')]:
  exe=Path(td)/(kind+'.exe')
  subprocess.run(['gcc','-std=c11','-O2','-DRUPTURE_BUILD','-Wno-unused-function','-I'+str(n),str(r/'tests/test_v179_fx_pcm.c'),str(src),str(n/'rupture_eq2.c'),'-lm','-o',str(exe)],check=True)
  audio=subprocess.check_output([str(exe)])
  out.append(hashlib.sha256(audio).hexdigest())
 assert out[0]=='93b98d02f77d868766cec5630a7b624133feebb7de0e2f37b55f6b54f30cea23'
 # V1.80+ changed the native reverb and limiter intentionally. This hash is
 # independently reproduced from the user's original complete V1.89 ZIP:
 # its native_fx.c linked with the same 780000-byte PCM fixture and eq2.c.
 assert out[1]=='b00a7be00fce2b491413438b11fff58f39c8bf1efaec9583dd0c76d6bbcd7d75',f'Historic FX baseline changed: {out}'
 # ROOM, HALL and DISTORTION are intentionally different as of V2.0.4.
 # This new golden guards accidental future audio changes without pretending
 # the edited effects are bit-identical to the earlier engines.
 assert out[2]=='584b1bd45db1667788b5dba7f052a1f240ec936afb8ef9b5a4333c2593e7851f',f'V2.0.4 FX PCM changed: {out}'
 print('PASS historic V1.78/V1.89 PCM and intentional V2.0.4 FX golden:',out)
