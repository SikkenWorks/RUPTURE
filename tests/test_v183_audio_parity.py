#!/usr/bin/env python3
"""Compare actual V1.82 mono core with V1.83 optimized core; no physical CPU claims."""
from pathlib import Path
import subprocess,re,statistics,hashlib
r=Path(__file__).resolve().parents[1]
src=r/'vendor/etch-v1.88-source/firmware/fm1-native'
others=[str(src/n) for n in ('native_fx.c','native_cxs_q15.c','rupture_eq2.c','wave_editor.c','native_performance.c','native_wave_fx.c')]
for bench in ('bench_v181_note_bursts.c',):
 for flags in (['-O2'],):
  results=[]
  for tag,core in (('v182',r/'reference/audio_core-V1.82.c'),('v183',src/'audio_core.c')):
   exe=r/'tests'/f'.{tag}-{bench}-{flags[0][1:]}'
   cmd=['gcc','-std=c11',flags[0],'-DRUPTURE_BUILD=1','-I'+str(src),str(r/'tests'/bench),str(core),*others,'-lm','-o',str(exe)]
   subprocess.run(cmd,check=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE,timeout=25)
   run=subprocess.check_output([str(exe)],text=True,timeout=8)
   results.append(run)
   exe.unlink()
  d=[re.search(r'(?:PCM_HASH|AUDIO_HASH)\s+([0-9a-f]{16})',x).group(1) for x in results]
  print(bench,flags[0],'V1.82',d[0],'V1.83',d[1],'MATCH',d[0]==d[1])
  if d[0]!=d[1]:raise AssertionError(results)
print('PASS all mono headroom/rapid MIDI/FX PCM comparisons')
