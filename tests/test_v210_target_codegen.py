#!/usr/bin/env python3
"""Optional pi32v2 guard; account for V2.0.27 FX dispatcher in SRAM."""
from pathlib import Path
import os
import re
import subprocess

root = Path(__file__).resolve().parents[1]
toolchain = Path(os.environ.get('JIELI_TOOLCHAIN', '/not-installed'))
objdump = toolchain / 'common/bin/objdump'
if not objdump.is_file():
    print('SKIP V2.0.10 pi32v2 instruction audit (set JIELI_TOOLCHAIN)')
    raise SystemExit(0)

build = root / 'vendor/etch-v1.88-source/build/fm1-native-storage-updater-uac48-p05'
assert (build / 'native_cxs_q15.o').is_file(), 'Run native build first'
assert (build / 'audio_core.o').is_file(), 'Run native build first'
assert (build / 'native_fx.o').is_file(), 'Run native build first'

def compiled_size(objname, symbol, section=None):
    listing = subprocess.check_output([str(objdump), '-t', str(build / objname)], text=True)
    target_section = section if section is not None else '.text.' + symbol
    matches = re.findall(r'^\w+\s+\w+\s+F\s+' + re.escape(target_section)
                         + r'\s+([0-9a-f]+)\s+' + re.escape(symbol) + r'$', listing, re.M)
    assert len(matches) == 1, f'No unique function {symbol} in {objname}'
    return int(matches[0], 16)

# V2.0.7 target baselines were audited before the V2.0.8 regression.
cxs = compiled_size('native_cxs_q15.o', 'rupture_q15_sample')
fx = compiled_size('native_fx.o', 'rupture_fx_process_ordered')
sub = compiled_size('audio_core.o', 'ru_sub_tick')
assert cxs == 478, f'CXS sustained exchange grew again: {cxs} (v207=478)'
release_title=(root/'README.md').read_text().splitlines()[0]
if 'V2.0.27' in release_title:
    # The public XIP dispatcher is now a tiny trampoline into unchanged SRAM
    # rendering math. Auditing its size alone would miss the actual DSP body.
    sram_fx = compiled_size('native_fx.o', 'rupture_fx_process_ordered_implementation', '.ram_text')
    assert 0 < fx <= 128, f'Unexpected public FX dispatcher size: {fx}'
    assert 7000 <= sram_fx <= 8192, f'SRAM FX render body changed size: {sram_fx}'
elif 'V2.0.28' in release_title or 'V2.0.29' in release_title or 'V2.0.30' in release_title or 'V2.0.31' in release_title or 'V2.0.32' in release_title or 'V2.0.33' in release_title or 'V2.0.34' in release_title or 'V2.0.35' in release_title or 'V2.0.36' in release_title or 'V2.0.37' in release_title or 'V2.0.38' in release_title or 'V2.0.39' in release_title or 'V2.0.40' in release_title or 'V2.0.43' in release_title:
    # V2.0.28+ restores all FX processing to XIP. Audit both
    # the small public wrapper and the substantial private processing body.
    xip_fx=compiled_size('native_fx.o','rupture_fx_process_ordered_implementation')
    assert 0 < fx <= 128, f'Unexpected FX wrapper size: {fx}'
    assert 8500 <= xip_fx <= 10000, f'FX body should be linked in XIP: {xip_fx}'
else:
    assert fx == 8392, f'EQ-only early return changed target footprint: {fx} (v210=8392)'
assert sub <= 858, f'SUB pitch path grew: {sub} (v207=968, v209=858)'
print(f'PASS native pi32v2: CXS={cxs} B, public FX dispatcher={fx} B, SUB={sub} B')
