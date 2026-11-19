#!/usr/bin/env python3
"""Fail if V2.0.27 FX hot path leaves SRAM or CXS source drifts."""
from pathlib import Path
import hashlib
import re
import subprocess

R = Path(__file__).resolve().parents[1]
S = R / 'vendor/etch-v1.88-source/firmware/fm1-native'

expected = {
    'native_cxs_q15.c': '86f4a39ad9bdbd6eabb79a34d9453e0fb0e150936667806e94eb2f00b57ca18f',
    'audio_core.c': '054b3791d142d998885039af1a362a0332d4ba9f6d538f47024c8548b830b7f5',
    'native_fx.h': '139cfb967b8bcff1ca1236cc0823a1c165ccec64c825c8767fb67bec9498223c',
    'rupture_subosc.h': '056a5cf04b79a7ab77966297748ced3eb529d17aae0af1fec28ff0d7b0bfb2e0',
    'native_fx.c': '098f48df81a1607ed8d3326f2b67b56162ec08b067f41bb2b8bee19d0a626292',
}
for name, digest in expected.items():
    actual = hashlib.sha256((S / name).read_bytes()).hexdigest()
    assert actual == digest, (name, actual, digest)

fx = (S / 'native_fx.c').read_text()
hot = (
    'etch_fx_ring_eq', 'rupture_sandpaper_process', 'ru_reverb_spring',
    'ru_reverb_room', 'ru_reverb_hall', 'etch_fx_wet_safety',
    'rupture_fx_process_ordered_implementation',
)
assert '#define RU_FX_FAST_TEXT __attribute__((section(".ram_text")))' in fx
for function in hot:
    assert re.search(r'RU_FX_FAST_TEXT[^\{;]+\b' + function + r'\s*\(', fx), function

uac_link = (S / 'app-uac.ld').read_text()
assert 'ASSERT(SIZEOF(.ram_text) <= 0x3400' in uac_link
assert '.ram_text : { _rt_start = .; KEEP(*(.ram_text*))' in uac_link
assert '> RAMTEXT AT > XIP' in uac_link

bringup = (S / 'bringup.c').read_text()
assert 'for(s=_rt_load,d=_rt_start;d<_rt_end;s++,d++)*d=*s;' in bringup
assert bringup.index('for(s=_rt_load,d=_rt_start;d<_rt_end;s++,d++)*d=*s;') < bringup.index('fm1_guard_enable(')
guard = (S / 'hal/fm1_guard.h').read_text()
assert 'extern uint32_t _rt_start[], _rt_end[];' in guard
assert 'uintptr_t)_rt_start' in guard and 'uintptr_t)_rt_end' in guard
builder = (S / 'build.py').read_text()
assert '-pi32v2-large-program=true' in builder

elf = R / 'vendor/etch-v1.88-source/build/fm1-native-storage-updater-uac48-p05/etch-native.elf'
if elf.is_file():
    sections = subprocess.check_output(['readelf', '-S', '-W', str(elf)], text=True)
    m = re.search(r'\]\s+\.ram_text\s+PROGBITS\s+([0-9a-f]+)\s+[0-9a-f]+\s+([0-9a-f]+)', sections)
    assert m, 'missing SRAM execution section'
    start, size = (int(v, 16) for v in m.groups())
    assert start == 0x01c00000, hex(start)
    assert 0 < size <= 0x3400, (size, 0x3400)
    symbols = subprocess.check_output(['nm', '-a', str(elf)], text=True)
    for function in hot:
        matches = [int(line.split()[0], 16) for line in symbols.splitlines()
                   if line.split() and line.split()[-1] == function]
        assert matches and all(start <= address < start + size for address in matches), (function, matches)
    print(f'PASS V2.0.27 SRAM ELF: {size} / 13312 B; {len(hot)} hot functions in SRAM')
else:
    print('NOTE ELF unavailable: static wiring checked; full SRAM placement requires native build')
print('PASS V2.0.27 exact CXS/audio source and startup/guard wiring')
