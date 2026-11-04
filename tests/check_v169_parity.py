#!/usr/bin/env python3
"""Re-run V1.69 host PCM + pixel checks against frozen V1.68 golden hashes.
No external paths, no hardware, no changes to firmware artifacts.
Requires GCC on Linux/Windows MSYS2. Host checksum parity is not a physical
FM1 timing or cross-OTA test.
"""
import pathlib,subprocess,tempfile
root=pathlib.Path(__file__).resolve().parents[1]
native=root/'vendor/etch-v1.88-source/firmware/fm1-native'
fixtures=root/'tests/fixtures'
def go(test,outbase,extra=()):
 with tempfile.TemporaryDirectory() as tmp:
  exe=pathlib.Path(tmp)/'test'
  cmd=['gcc','-std=c11','-O2','-DRUPTURE_BUILD=1','-I'+str(native),str(root/'tests'/test),*map(str,extra),'-lm','-o',str(exe)]
  subprocess.run(cmd,check=True)
  return subprocess.check_output([str(exe),*(['24000'] if 'matrix' in test else [])],text=True)
for t, fixture, extra in [
 ('test_v169_frame_checksum.c','v168_render_checksums.txt',()),
 ('test_v169_fx_pcm.c','v168_fx_pcm_checksums.txt',(native/'native_fx.c',native/'rupture_eq2.c')),
 ('test_v140_fx_matrix.c','v168_fx_matrix_checksums.txt',(native/'native_fx.c',native/'rupture_eq2.c'))]:
 expected=(fixtures/fixture).read_text()
 actual=go(t,fixture,extra)
 assert actual==expected,f'Audio/UI changed versus golden V1.68: {t}'
 print('PASS byte-exact golden',t)
