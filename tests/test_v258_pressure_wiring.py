#!/usr/bin/env python3
"""RUPTURE V2.0.58: digital keys, LED sweep and CXS are not analog-owned."""
from pathlib import Path
import hashlib

root=Path(__file__).resolve().parents[1]
src=root/'vendor/etch-v1.88-source/firmware/fm1-native'
read=lambda name:(src/name).read_text()
sha=lambda name:hashlib.sha256((src/name).read_bytes()).hexdigest()
scanner=read('rupture_touch_adc.h')
bringup=read('bringup.c')
target=read('audio_target.c')
ui=read('rupture_ui.c')
storage=read('wave_store.c')
assert sha('native_cxs_q15.c')=='86f4a39ad9bdbd6eabb79a34d9453e0fb0e150936667806e94eb2f00b57ca18f'
assert sha('native_fx.c')=='d7a7b55f99011d92a98f8ce8f20a6d2ec3374fc8f67fdcac48c70a05d248412a'
assert sha('audio_core.c')=='0e5588d24d41f69be5e3065acf4218795a4b55916f448a1b595d984fde7eb235'
assert sha('audio_core_core2.c')=='a7302c0c51fadfa695cee5fc2875cdaad37443a35bf957ee40f41f00fd9002bd'
assert bringup.index('ru_touch_adc_before_scan();')<bringup.index('fm1_input_tick();')
assert bringup.index('fm1_input_tick();')<bringup.index('ru_touch_adc_after_scan(nested);')
assert 'FM1_PR(FM1_PA,FM1_DIR)|=m;' in scanner
assert scanner.index('FM1_PR(FM1_PA,FM1_DIR)|=m;')<scanner.index('if(FM1_ADC_CON&0x80u)')
assert 'fm1_touch_hw_on=0u;fm1_touch_hw_fault=1u;' in scanner
assert 'if(!fm1_touch_hw_on||fm1_touch_pending||audio_nested)return;' in scanner
assert 'if(id>=14&&id<34&&(fm1_in.notes&(1u<<(id-14))))' in scanner
assert 'ru_touch_apply_profile(&c,&touch_profile,expr_level);' in target
assert target.count('ru_touch_apply_profile(')==1
assert 'ru_touch_live_pressure=ru_touch_update(' in bringup
assert 'ru_touch_adc_snapshot(samples,generations)' in bringup
assert 'if(ru_settings_selected==8u)' in bringup
assert '"KEY PRESSURE"' in ui and '"GROWL"' in ui
assert 'v.touch_target=p->target' in bringup
assert 'ru_touch_edit_engine' in bringup and 'ru_touch_edit_field' in bringup
assert 'ru_touch_motion(&ru_touch_drift,touch_profile' in bringup
assert 'etch_store_touch_profile_save(&native_store_io,engine' in bringup
assert 'ru_touch_ensure_reverb(chain,p->min_pct)' in bringup
assert 'RU_TOUCH_PREF_MAGIC 0x7000u' in storage
assert 'RU_EXPR_PREF_MAGIC 0x7400u' in storage
assert 'ru_touch_pref_magic(rec.magic)' in storage
assert 'ru_preset_assign_pair_make_touch(mapping,growl,usb_rate48,touch)' in storage
assert 'etch_store_touch_save(&native_store_io,ru_touch_conf)' in bringup
assert 'ru_boot_intro_begin();' in bringup
assert 'ru_led_boot_wave_level' in bringup
print('PASS V2.0.58 independent CXS/GROWL touch wiring, auto-reverb, protected ADC note ownership')
