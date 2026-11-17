#!/usr/bin/env python3
"""Optional actual pi32v2 instruction footprint guard (needs Jieli tools)."""
from pathlib import Path
import os
import re
import subprocess

root = Path(__file__).resolve().parents[1]
toolchain = Path(os.environ.get('JIELI_TOOLCHAIN', '/not-installed'))
objdump = toolchain / 'common/bin/objdump'
if not objdump.is_file():
    print('SKIP V2.0.9 pi32v2 instruction audit (set JIELI_TOOLCHAIN)')
    raise SystemExit(0)

build = root / 'vendor/etch-v1.88-source/build/fm1-native-storage-updater-uac48-p05'
assert (build / 'native_cxs_q15.o').is_file(), 'Run native build first'
assert (build / 'audio_core.o').is_file(), 'Run native build first'
assert (build / 'native_fx.o').is_file(), 'Run native build first'

def compiled_size(objname, symbol):
    listing = subprocess.check_output([str(objdump), '-t', str(build / objname)], text=True)
    matches = re.findall(r'^\w+\s+\w+\s+F\s+\.text\.' + re.escape(symbol)
                         + r'\s+([0-9a-f]+)\s+' + re.escape(symbol) + r'$', listing, re.M)
    assert len(matches) == 1, f'No unique function {symbol} in {objname}'
    return int(matches[0], 16)

# V2.0.7 target baselines were audited before the V2.0.8 regression.
cxs = compiled_size('native_cxs_q15.o', 'rupture_q15_sample')
fx = compiled_size('native_fx.o', 'rupture_fx_process_ordered')
sub = compiled_size('audio_core.o', 'ru_sub_tick')
assert cxs == 478, f'CXS sustained exchange grew again: {cxs} (v207=478)'
assert fx <= 8156, f'FX processing grew: {fx} (v207=8368, v209=8156)'
assert sub <= 858, f'SUB pitch path grew: {sub} (v207=968, v209=858)'
print(f'PASS V2.0.9 native pi32v2: CXS={cxs} B, FX={fx} B (v207=8368 B), SUB={sub} B (v207=968 B)')
