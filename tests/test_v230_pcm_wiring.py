#!/usr/bin/env python3
"""Guard the packed 48k FIR, scope gate and unchanged CXS/reverb math."""
from pathlib import Path
import hashlib
import re
import subprocess

root=Path(__file__).resolve().parents[1]
src=root/'vendor/etch-v1.88-source/firmware/fm1-native'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
for name, expected in {
 'native_cxs_q15.c':'86f4a39ad9bdbd6eabb79a34d9453e0fb0e150936667806e94eb2f00b57ca18f',
 'audio_core.c':'054b3791d142d998885039af1a362a0332d4ba9f6d538f47024c8548b830b7f5',
 'native_fx.c':'d7a7b55f99011d92a98f8ce8f20a6d2ec3374fc8f67fdcac48c70a05d248412a',
 'audio_target.c':'8f54b06c4cec5470912fbc9995c36f7198eeb306e7161ffc55c3dd1f97600f6f',
 'uac_fir.h':'063535c30132a6731d01975ed35c39cb78c33f9a4aef825ce943f9999e4b5f1b',
 'usb.c':'8416402a8159d119556fa3255eecc755a583226c95c73086da197b37e82cf458',
 'bringup.c':'72916436fa59ce34b468b4d21640df2dc07723189a06083172afe6cb67eb16a7',
}.items():
 actual=sha(src/name)
 assert actual==expected,(name,actual,expected)

usb=(src/'usb.c').read_text()
start=usb.index('static __attribute__((noinline)) void uac_tap48(')
end=usb.index('\n}\n',start)+3
fir=usb[start:end]
assert hashlib.sha256(fir.encode()).hexdigest()=='ad8cda018cd66173aa34c94e530998054e2561478b4ae6deb00a6fa56ff613ac'
assert '#define RU_UAC_STAGE_N 1024u' in usb
assert 'ru_uac_stage_push(out,n);' in usb
assert 'uac_tap48(ru_uac_block,HALF_FRAMES);' in usb
assert 'ru_uac_block[i]=packed;' in usb
assert 'uint32_t lr = packed[i];' in fir
assert 'ru_uac_unpacked' not in usb
assert 'int16_t l = (int16_t)(lr & 0xffffu)' in fir
assert 'static __attribute__((noinline)) void uac_foreground_service(void)' in usb
assert 'if (uac.r48) {' in usb and 'if (!uac.go || uac.ring48 != uac.r48) return;' in usb
assert 'ua_ring[(w + i) & (UA_N - 1u)]' in usb # 44.1 kHz direct path preserved
assert 'RING_PUBLISH();\n    ru_uac_stage_w=w+n;' in usb
assert 'RING_PUBLISH();\n    ru_uac_stage_r=r+HALF_FRAMES;' in usb
assert 'if(!native_updating)uac_foreground_service();' in (src/'bringup.c').read_text()
assert 'RATE 48K FOREGROUND' in (src/'bringup.c').read_text()
assert 'RATE 44.1K DIRECT' in (src/'bringup.c').read_text()
assert '#define RU_FX_SRAM_HOT 0' in (src/'native_fx.c').read_text()
audio=(src/'audio_target.c').read_text()
assert 'uint8_t prepare_scope=(uint8_t)(!native_midi_backlog&&native_ui_pressure<16u);' in audio
assert 'if(prepare_scope)native_mono[i]' in audio
assert 'if(prepare_scope &&' in audio

elf=root/'vendor/etch-v1.88-source/build/fm1-native-storage-updater-uac48-p05/etch-native.elf'
if elf.is_file():
 sections=subprocess.check_output(['readelf','-S','-W',str(elf)],text=True)
 m=re.search(r'\]\s+\.uac_ram\s+NOBITS\s+([0-9a-f]+)\s+[0-9a-f]+\s+([0-9a-f]+)',sections)
 assert m,'UAC SRAM missing'
 address,size=(int(x,16) for x in m.groups())
 assert address==0x01c04000
 assert size==0x1bcc and size<=0x2000,(address,size)
 syms=subprocess.check_output(['nm','-a',str(elf)],text=True,stderr=subprocess.DEVNULL)
 assert ' uac_foreground_service' in syms and ' ru_uac_stage_push' in syms
 print(f'PASS V2.0.30 UAC SRAM: {size}/8192 B, 48k FIR outside ALNK0')
else:print('NOTE ELF absent: static USB/FX wiring checked')
print('PASS V2.0.30 packed FIR, optional scope, CXS source invariants; 44.1k retained')
