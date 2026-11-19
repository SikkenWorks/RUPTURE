#!/usr/bin/env python3
"""Guard the high-risk live FIR as an opt-in, non-default build only."""
from pathlib import Path
import hashlib
import re

root=Path(__file__).resolve().parents[1]
src=root/'vendor/etch-v1.88-source/firmware/fm1-native'
sha=lambda name:hashlib.sha256((src/name).read_bytes()).hexdigest()
for name,expected in {
 'native_cxs_q15.c':'86f4a39ad9bdbd6eabb79a34d9453e0fb0e150936667806e94eb2f00b57ca18f',
 'native_fx.c':'d7a7b55f99011d92a98f8ce8f20a6d2ec3374fc8f67fdcac48c70a05d248412a',
 'audio_target.c':'b5af317243517c50e9b44b95c517f3d920282e53bbbeb4440bc652e19d57599c',
 'audio_core.c':'6499bf5d022ac8e61457d52a7400f109794c5d5ae790d6ec3899972bcd1062c9',
}.items():assert sha(name)==expected,(name,sha(name))
build=(src/'build.py').read_text()
lab=(src/'rupture_cpu1_lab.h').read_text()
usb=(src/'usb.c').read_text()
core=(src/'rupture_cpu1_uac_block.h').read_text()
assert 'ap.add_argument("--dual-core-live-uac-lab"' in build
assert 'requires --dual-core-lab' in build
assert "-c1live' if args.dual_core_live_uac_lab" in build
assert '-DETCH_CPU1_UAC_LIVE_LAB=' in build
assert 'if(ru_cpu1_uac_live_poll())return;' in usb
assert 'ru_cpu1_uac_live_submit(ru_uac_block,n)' in usb
assert 'uac_tap48(ru_uac_block,n);' in usb
assert 'if(!ru_cpu1_uac_verify_envelope(' in lab
assert 'ru_cpu1_uac_expected_one(' in lab
assert 'RU_C1_UAC_PROBATION 64u' in lab
assert '((ru_c1_audio_seen&255u)==255u)' in lab
assert 'ru_cpu1_uac_apply_history(' in lab
assert 'ru_cpu1_retire(RU_C1_TIMEOUT)' in lab
assert 'ru_c1_audio_box.seq=seq;' in lab
assert 'RU_C1_UAC_IN 64u' in core
assert 'CPU1' not in (src/'native_cxs_q15.c').read_text()
print('PASS V2.0.34: 48k CPU1 offload guarded and opt-in; CXS/FX unchanged; V243 note guard pinned')
