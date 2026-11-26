#!/usr/bin/env python3
"""Final CORE2 click-zero fast path remains after exact bounded post-gain."""
from pathlib import Path
root=Path(__file__).resolve().parents[1]
core=(root/'vendor/etch-v1.88-source/firmware/fm1-native/audio_core_core2.c').read_text()
core0=(root/'vendor/etch-v1.88-source/firmware/fm1-native/audio_core.c').read_text()
finish=core.split('static void ru_fx_c2_finish(',1)[1].split('\n}\n',1)[0]
assert 'if(post<=32768){' in finish
assert 'l=(l*post)/32768;' in finish
assert 'r=(r*post)/32768;' in finish
assert 'l=clamp15((l*post)/32768);' in finish
assert 'r=clamp15((r*post)/32768);' in finish
assert 'if(v->click){' in finish
assert 'out[f*2]=(int16_t)clamp15(l+v->click);' in finish
assert 'out[f*2+1]=(int16_t)clamp15(r+v->click);' in finish
assert 'out[f*2]=(int16_t)l;' in finish
assert 'out[f*2+1]=(int16_t)r;' in finish
assert finish.index('v->post_gain')<finish.index('if(v->click)')
assert 'ru_fx_c2_finish(' not in core0,'STABLE renderer should not use this offload path'
assert 'if(ru_c2_fx&&!ru_metronome_enabled)ru_fx_c2.samples[f].click=0;' in core
print('PASS V2.0.58 CORE2 click-zero and bounded-gain fast paths retained')
