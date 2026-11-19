#!/usr/bin/env python3
"""Safety boundaries for V2.0.35 experimental CPU1 audio guard."""
from pathlib import Path
import hashlib

root=Path(__file__).resolve().parents[1]
src=root/'vendor/etch-v1.88-source/firmware/fm1-native'
lab=(src/'rupture_cpu1_lab.h').read_text()
usb=(src/'usb.c').read_text()
boot=(src/'bringup.c').read_text()
build=(src/'build.py').read_text()
guard=(src/'rupture_cpu1_offload_guard.h').read_text()
for name,expected in {
 'native_cxs_q15.c':'86f4a39ad9bdbd6eabb79a34d9453e0fb0e150936667806e94eb2f00b57ca18f',
 'native_fx.c':'d7a7b55f99011d92a98f8ce8f20a6d2ec3374fc8f67fdcac48c70a05d248412a',
 'audio_target.c':'b5af317243517c50e9b44b95c517f3d920282e53bbbeb4440bc652e19d57599c',
 'audio_core.c':'6499bf5d022ac8e61457d52a7400f109794c5d5ae790d6ec3899972bcd1062c9',
}.items():
 assert hashlib.sha256((src/name).read_bytes()).hexdigest()==expected,name
assert 'RU_C1_UAC_MIN_RING_FRAMES' in guard
assert 'RU_C1_PRESS_DMA_LATE' in guard and 'RU_C1_PRESS_USB_UND' in guard
assert 'RU_C1_PRESS_AUDIO_STOP' in guard and 'RU_C1_PRESS_USB_STAGE' in guard
assert 'ru_cpu1_uac_pressure_watch(' in lab
assert 'ru_cpu1_pressure_update(&ru_c1_pressure' in lab
assert 'ru_cpu1_retire(RU_C1_PRESSURE)' in lab
assert 'if(!ru_c1_online){' in lab
assert 'ru_cpu1_offload_decide(' in lab
assert 'ua_w-ua_r' in lab
assert 'ru_c1_audio_ring_aborts++' in lab
assert 'ru_c1_audio_max_wait_ticks=dt' in lab
assert 'if(!native_updating)uac_foreground_service();' in boot
assert 'native_late,native_dma_blocked,uac.underruns,ru_uac_stage_drops' in boot
assert '((uac.flowing&&uac.go)||native_tripped||ru_c1_audio_pending)' in boot
assert 'label("C2 MAX US"' in boot and 'label("C2 TRIP"' in boot
assert 'if(ru_cpu1_uac_live_poll())return;' in usb
assert 'ru_cpu1_uac_live_submit(ru_uac_block,n)' in usb
assert 'args.dual_core_live_uac_lab' in build
assert 'cpu1_live_fail_closed_guard_v235' in build
assert 'RUPTURE V2.0.43' in build
print('PASS V2.0.35: guarded live path opt-in, CXS/FX exact; V243 note handling pinned')
