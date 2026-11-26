#!/usr/bin/env python3
"""CPU1 lab is opt-in, source PCM is byte-identical, memory/stack bounds."""
from pathlib import Path
import hashlib
import re
import subprocess
root=Path(__file__).resolve().parents[1]
src=root/'vendor/etch-v1.88-source/firmware/fm1-native'
sha=lambda name:hashlib.sha256((src/name).read_bytes()).hexdigest()
for name,expected in {
 'native_cxs_q15.c':'86f4a39ad9bdbd6eabb79a34d9453e0fb0e150936667806e94eb2f00b57ca18f',
 'native_fx.c':'d7a7b55f99011d92a98f8ce8f20a6d2ec3374fc8f67fdcac48c70a05d248412a',
 'audio_target.c':'3043710cebb5de5096b22eb2a178585ab6b75d69648b7e085da7a37cb81405ea',
 'audio_core.c':'0e5588d24d41f69be5e3065acf4218795a4b55916f448a1b595d984fde7eb235',
}.items():
 assert sha(name)==expected,(name,sha(name),expected)
build=(src/'build.py').read_text()
boot=(src/'bringup.c').read_text()
cpu=(src/'rupture_cpu1_lab.h').read_text()
link=(src/'app-uac.ld').read_text()
assert 'ap.add_argument("--dual-core-lab",action="store_true"' in build
assert 'requires --usb-audio-48k' in build
assert "if args.dual_core_lab else 'hal/fm1_vec.S'" in build
assert 'ETCH_CPU1_LAB' in boot
assert 'fm1_input_scan();\n fm1_input_scan();' in boot
assert 'if(!safe && !(fm1_in.buttons&((1u<<13)|1u)))' in boot
assert 'ru_cpu1_diag_service(etch_ms,' in boot
assert 'ru_c1_online?"CORE2 LAB OK":"CORE2 RETIRED"' in boot
assert 'ru_cpu1_halt(); /* prior firmware/warm reset' in boot
assert '#define RU_C1_GUARD' in (src/'rupture_cpu1_protocol.h').read_text()
assert 'ru_cpu1_verify(' in cpu and 'ru_cpu1_retire(RU_C1_BAD_OUTPUT)' in cpu
assert 'ru_cpu1_retire(RU_C1_TIMEOUT)' in cpu and 'ru_c1_boot.barred=1u' in cpu
assert 'ru_cpu1_fault_dispatch(uint32_t *f)' in cpu
assert 'for(uint32_t n=0u;n<32u;n++)__asm__ volatile("nop")' in cpu
assert 'C1_RAM  (rw) : ORIGIN = 0x01C06000, LENGTH = 0x2000' in link
assert 'ru_cpu1_entry:' in (src/'hal/rupture_cpu1_entry.S').read_text()
elf=src.parents[1]/'build/fm1-native-storage-updater-uac48-p05-c1lab/etch-native.elf'
if elf.is_file():
 sections=subprocess.check_output(['readelf','-S','-W',str(elf)],text=True)
 m=re.search(r'\]\s+\.c1_ram\s+NOBITS\s+([0-9a-f]+)\s+[0-9a-f]+\s+([0-9a-f]+)',sections)
 assert m
 address,size=(int(x,16) for x in m.groups())
 assert address==0x01c06000 and size<=8192,(address,size)
 print(f'PASS CPU1 isolated RAM: {size}/8192 B')
print('PASS V2.0.32 optional CPU1 integrity lab; CXS, FX, audio DMA and USB FIR unchanged')
