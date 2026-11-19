#!/usr/bin/env python3
"""Verify FX timing rollback, CXS integrity and warmup gate on target ELF."""
from pathlib import Path
import hashlib
import re
import subprocess

root = Path(__file__).resolve().parents[1]
src = root / 'vendor/etch-v1.88-source/firmware/fm1-native'
sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
expected = {
    'native_cxs_q15.c': '86f4a39ad9bdbd6eabb79a34d9453e0fb0e150936667806e94eb2f00b57ca18f',
    'audio_core.c': '054b3791d142d998885039af1a362a0332d4ba9f6d538f47024c8548b830b7f5',
    'rupture_subosc.h': '056a5cf04b79a7ab77966297748ced3eb529d17aae0af1fec28ff0d7b0bfb2e0',
    'native_fx.h': '139cfb967b8bcff1ca1236cc0823a1c165ccec64c825c8767fb67bec9498223c',
    'native_fx.c': 'd7a7b55f99011d92a98f8ce8f20a6d2ec3374fc8f67fdcac48c70a05d248412a',
    'audio_target.c': '021915e3799581e8b184e896ebc631634dfff942bceebb38fe82b1a9c776efb3',
}
for name, digest in expected.items():
    actual = sha(src / name)
    assert actual == digest, f'{name}: {actual} != {digest}'

fx = (src / 'native_fx.c').read_text()
assert '#define RU_FX_SRAM_HOT 0' in fx
assert '((l-dry_l)*fade)/32767' in fx
assert '((r-dry_r)*fade)/32767' in fx
assert '#if defined(RU_FX_FADE_64_REF) && RU_FX_FADE_64_REF' in fx
builder = (src / 'build.py').read_text()
assert '-pi32v2-large-program=true' not in builder
assert "'version':'RUPTURE V2.0.28'" in builder

target = (src / 'audio_target.c').read_text()
assert 'render_keys=ru_note_keys_while_output_gated(keys,native_output_enabled)' in target
assert 'etch_native_render_stereo(&native_audio,&c,render_keys,native_stereo,ETCH_NATIVE_FRAMES)' in target
assert 'native_output_enabled?1u:0u' in target

names = (
    'etch_fx_ring_eq', 'rupture_sandpaper_process', 'ru_reverb_spring',
    'ru_reverb_room', 'ru_reverb_hall', 'etch_fx_wet_safety',
    'rupture_fx_process_ordered_implementation',
)
elf = root / 'vendor/etch-v1.88-source/build/fm1-native-storage-updater-uac48-p05/etch-native.elf'
if elf.is_file():
    sections = subprocess.check_output(['readelf', '-S', '-W', str(elf)], text=True)
    match = re.search(r'\]\s+\.ram_text\s+PROGBITS\s+([0-9a-f]+)\s+[0-9a-f]+\s+([0-9a-f]+)', sections)
    assert match, 'SRAM text missing'
    start, size = (int(x, 16) for x in match.groups())
    assert start == 0x01c00000 and 0 < size < 0x3400, (start, size)
    syms = subprocess.check_output(['nm', '-a', str(elf)], text=True, stderr=subprocess.DEVNULL)
    for name in names:
        matches = [int(line.split()[0], 16) for line in syms.splitlines()
                   if line.split() and line.split()[-1] == name]
        assert matches and all(0x02000000 <= address < 0x02100000 for address in matches), (name, matches)
    print(f'PASS V2.0.28 ELF: {len(names)} FX hot routines back in XIP; SRAM text {size} B')
else:
    print('NOTE native ELF unavailable; static wiring verified')
print('PASS V2.0.28 CXS/core source unchanged; initial key gate and exact FX fade wired')
