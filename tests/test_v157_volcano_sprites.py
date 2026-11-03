"""Legacy regression: adapted in V1.65 to intentionally moving V1.58+ sprites.
The former unshifted-screen pixel assertions were invalid after shake was added.
"""
from pathlib import Path
import runpy,subprocess,tempfile
r=Path(__file__).resolve().parents[1]
runpy.run_path(str(r/'tests/test_v165_png_parity.py'))
with tempfile.TemporaryDirectory() as d:
 out=Path(d)/'native_sprite_test'
 subprocess.run(['gcc','-O2','-Wno-unused-function',str(r/'tests/test_v165_sprite_mapping.c'),'-lm','-o',str(out)],check=True)
 subprocess.run([str(out)],check=True)
print('PASS: legacy PNG test now accounts for intentional UI shake')
