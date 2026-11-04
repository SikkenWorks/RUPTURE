#!/usr/bin/env python3
"""Compare original V1.80 and optimized V1.81 realtime audio host PCM; no hardware claims."""
from pathlib import Path
import subprocess,re,statistics,os
root=Path(__file__).resolve().parents[1]
src=root/'vendor/etch-v1.88-source/firmware/fm1-native'
common=[str(src/n) for n in ['native_fx.c','native_cxs_q15.c','rupture_eq2.c','wave_editor.c','native_performance.c','native_wave_fx.c']]
bench=str(root/'tests/bench_v181_note_bursts.c')
results=[]
for label,core in [('v180',root/'reference/audio_core-V1.80.c'),('v181',src/'audio_core.c')]:
 out='/tmp/rupture_'+label+'_pcm'
 cmd=['gcc','-std=c11','-O2','-DRUPTURE_BUILD=1','-I'+str(src),bench,str(core),*common,'-lm','-o',out]
 subprocess.run(cmd,check=True,capture_output=True)
 runs=[subprocess.check_output([out],text=True).strip() for _ in range(5)]
 hashes={re.search(r'PCM_HASH ([0-9a-f]+)',s).group(1) for s in runs}
 assert len(hashes)==1,(label,runs)
 timings=[float(re.search(r'TIME_MS ([\d.]+)',s).group(1)) for s in runs]
 results.append((label,hashes.pop(),statistics.median(timings)))
print('HOST PCM PARITY',results)
assert results[0][1]==results[1][1], 'V1.80 audio divergence'
print('PASS V1.81 mono fast-note PCM bit identity; host median elapsed ms',results[0][2],results[1][2])
