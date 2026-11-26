#!/usr/bin/env python3
"""Faster intro must not remove or consume the 41-key keyboard LED wave."""
from pathlib import Path

root=Path(__file__).resolve().parents[1]
native=root/'vendor/etch-v1.88-source/firmware/fm1-native'
bringup=(native/'bringup.c').read_text()
leds=(native/'rupture_hw_leds.h').read_text()
timeline=(native/'rupture_intro_timeline.h').read_text()
cstart=bringup.split('void fm1_cstart(void)',1)[1]

assert '#define RU_LED_BOOT_WAVE_STEP_MS 27u' in leds
assert '#define RU_LED_BOOT_WAVE_TOTAL_MS (41u*RU_LED_BOOT_WAVE_STEP_MS+RU_LED_BOOT_WAVE_TAIL_MS)' in leds
assert 'ru_led_boot_wave_level(boot_wave_ms,(uint8_t)k,level)' in bringup
assert 'ru_led_boot_wave_level(boot_wave_ms,(uint8_t)(27u+b),level)' in bringup
assert cstart.index('ru_boot_intro_begin();') < cstart.index('native_compile_edit();')
assert cstart.index('native_compile_edit();') < cstart.index('native_start();')
assert cstart.index('native_seq_led_refresh();') < cstart.index('ru_splash_done=1u;')
assert 'ru_boot_led_last_ms=etch_ms;native_seq_led_refresh();' in cstart
assert cstart.index('ru_splash_done=1u;') < cstart.index('screen();')
assert 'native_output_poll();' in cstart
assert 'if(boot_audio&&!native_updating)uac_foreground_service();' in cstart
assert 'if(boot_audio)ru_cpu1_diag_service(etch_ms,' in cstart
assert 'etch_output_screen_advance(&native_screen_gate,fm1_ticks()' in cstart
assert 'if(!ru_led_started){ru_led_start_ms=etch_ms;ru_led_started=1u;}' in bringup
assert '#define RU_INTRO_LOGO_IN 7u' in timeline
assert '#define RU_INTRO_LOGO_OUT 7u' in timeline
assert '#define RU_INTRO_TEXT_IN 8u' in timeline
assert '#define RU_INTRO_TEXT_OUT 8u' in timeline
print('PASS V2.0.58: 5.16s intro overlaps engine/codec init, original 41-key LED wave preserved')
