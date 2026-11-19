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
assert cstart.index('ru_boot_splash();ru_splash_done=1u;') < cstart.index('native_seq_led_refresh();')
assert 'if(!ru_led_started){ru_led_start_ms=etch_ms;ru_led_started=1u;}' in bringup
assert '#define RU_INTRO_LOGO_IN 7u' in timeline
assert '#define RU_INTRO_LOGO_OUT 7u' in timeline
assert '#define RU_INTRO_TEXT_IN 8u' in timeline
assert '#define RU_INTRO_TEXT_OUT 8u' in timeline
print('PASS V2.0.43: faster 5.16s fades, original 41-key LED wave starts after intro')
