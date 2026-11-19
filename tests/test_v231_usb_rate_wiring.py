#!/usr/bin/env python3
"""Guard reversible USB-only V2.0.31 changes and no CXS/FX PCM mutation."""
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
    'audio_target.c':'b5af317243517c50e9b44b95c517f3d920282e53bbbeb4440bc652e19d57599c',
    'uac_fir.h':'063535c30132a6731d01975ed35c39cb78c33f9a4aef825ce943f9999e4b5f1b',
    'audio_core.c':'6499bf5d022ac8e61457d52a7400f109794c5d5ae790d6ec3899972bcd1062c9',
}.items():
    assert sha(name)==expected,(name,sha(name),expected)
usb=(src/'usb.c').read_text()
ui=(src/'rupture_ui.c').read_text()
boot=(src/'bringup.c').read_text()
store=(src/'wave_store.c').read_text()
header=(src/'rupture_usb_rate_descriptor.h').read_text()
assert '#include "rupture_usb_rate_descriptor.h"' in usb
assert 'ru_uac_restrict_format(CFG_DESC_PLAIN,sizeof(CFG_DESC_PLAIN),ru_uac_rate48);' in usb
assert 'ru_uac_restrict_format(CFG_DESC,sizeof(CFG_DESC),ru_uac_rate48);' in usb
assert 'cfg[at+7u]=0u;' in header and 'cfg[at+11u+i]=octet;' in header
assert 'DEV_DESC_PLAIN[12]=' in usb and 'DEV_DESC[12]=' in usb
assert 'static void ru_uac_rate_switch(uint8_t rate48)' in usb
assert 'fm1_usb_off();' in usb[usb.index('static void ru_uac_rate_switch'):usb.index('\n}\n',usb.index('static void ru_uac_rate_switch'))]
assert 'if(hz!=(ru_uac_rate48?48000u:UAC_RATE))return;' in usb
assert 'if (request_hz!=(ru_uac_rate48?48000u:UAC_RATE)) {' in usb
assert 'if (usb.up || usb.detached || now_ms - t < 1000u)' in usb
assert 'uac_tap48(ru_uac_block,n);' in usb
assert 'uint32_t lr = packed[i];' in usb
assert 'ua_ring[(w + i) & (UA_N - 1u)]' in usb
assert 'if(!native_updating)uac_foreground_service();' in boot
assert 'USB-C AUDIO' in ui and '44.1 KHZ' in ui and '48.0 KHZ' in ui
assert 'USB RECONNECTS ON CHANGE' in ui and 'V2.0.39' in ui
assert 'ru_settings_selected+select,0,7' in boot
assert 'ru_uac_rate_switch(chosen);' in boot and 'ru_uac_rate_switch(preferred);' in boot
assert 'etch_store_usb_rate_load' in boot and 'etch_store_usb_rate_save' in boot
assert 'if(ru_usb_rate_dirty)' in boot and '!native_blend_audio_busy()' in boot
assert 'ru_usb_rate_dirty=1u;' in boot and 'ru_global_gain_changed_ms' in boot
assert 'ru_preset_assign_pair_make' in store and 'rate48?128u:0u' in store
assert 'rec.opposite&127u' in store and 'etch_store_usb_rate_load' in store
elf=root/'vendor/etch-v1.88-source/build/fm1-native-storage-updater-uac48-p05/etch-native.elf'
if elf.is_file():
    sections=subprocess.check_output(['readelf','-S','-W',str(elf)],text=True)
    m=re.search(r'\]\s+\.uac_ram\s+NOBITS\s+([0-9a-f]+)\s+[0-9a-f]+\s+([0-9a-f]+)',sections)
    assert m
    address,size=(int(x,16) for x in m.groups())
    assert address==0x01c04000 and size<=8192,(address,size)
    print(f'PASS V2.0.31 UAC SRAM budget: {size}/8192 B')
print('PASS V2.0.31 USB-only rate switch and persistent preference, CXS/FX sample math unchanged')
