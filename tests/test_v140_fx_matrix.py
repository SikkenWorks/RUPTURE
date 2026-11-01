#!/usr/bin/env python3
"""Golden PCM hashes from the unmodified RUPTURE V1.39 engine.
The V1.40 optimized dispatch must be sample-exact for each scenario.
Host checks do not assert physical FM1 DMA headroom.
"""
from pathlib import Path
import subprocess
root=Path(__file__).resolve().parents[1]
p=root/'vendor/etch-v1.88-source/firmware/fm1-native'
target=root/'tests/test_v140_fx_matrix.c'
exe=Path('/tmp/rupture_v140_fx_matrix_release')
subprocess.run(['gcc','-O2','-DRUPTURE_BUILD',str(target),str(p/'native_fx.c'),str(p/'rupture_eq2.c'),'-o',str(exe)],check=True)
expected=[
(0,3245008337,0),(1,248154167,0),(2,3505961632,-1516),
(3,3440934559,303),(4,3323543160,-37735),(5,3245008337,0),
(6,3223674915,0),(7,793193779,0),(8,172894362,157639),
(9,2467605651,-17331),(10,2462110169,114678),(11,465276511,18645)]
rows=[tuple(map(int,x.split())) for x in subprocess.check_output([str(exe),'40000'],text=True).splitlines()]
assert rows==expected,(rows,expected)
s=(p/'native_fx.c').read_text()
assert 'if(chorus_render){' in s and 'if(delay_render){' in s and 'if(reverb_target || s->reverb_mix_smooth){' in s
assert 'if(!(bypass_mask&ETCH_FXB_BIT(ETCH_FXB_DELAY))||delay_render)' not in s
b=(p/'bringup.c').read_text()
assert 'native_fx_bypass_mask|=ETCH_FXB_BIT(ETCH_FXB_EQ);' in b
print('PASS: 12 x 40000 stereo PCM scenarios bit-for-bit identical to V1.39; disabled time FX no wet DSP; EQ bypass NEW project')
