"""V2.0.58: physical PRESS visibility and reverb MIN/live consistency."""
from pathlib import Path

root = Path(__file__).resolve().parents[1]
native = root / 'vendor/etch-v1.88-source/firmware/fm1-native'
adc = (native/'rupture_touch_adc.h').read_text()
touch = (native/'rupture_key_touch.h').read_text()
bringup = (native/'bringup.c').read_text()
ui = (native/'rupture_ui.c').read_text()

# The old divider 15 repeatedly missed the 100-us conversion window;
# DRIVE 2 distinguishes carbon-piIl contact force without changing scans.
assert 'FM1_ADC_CON=0x2047u' in adc
assert 'FM1_PR(FM1_PA,FM1_HD)|=m' in adc
assert 'ru_touch_adc_before_scan();' in bringup
assert 'fm1_input_tick();' in bringup
assert 'ru_touch_adc_after_scan(nested);' in bringup
assert bringup.index('ru_touch_adc_before_scan();') < bringup.index('fm1_input_tick();') < bringup.index('ru_touch_adc_after_scan(nested);')
assert 's->raw=sample;' in touch
assert 'if(sample>s->landing)s->landing=sample;' in touch

# SPEED only exists for DRIFT/RANDOM; PRESS gets live raw/sensor status.
assert '"SENSOR RAW"' in ui and '"MIDI SOURCE"' in ui
assert 'changed.mode==RU_TOUCH_MODE_DRIFT||changed.mode==RU_TOUCH_MODE_RANDOM' in bringup
assert '"ADC ERR"' in ui
assert 'v.touch_raw' in bringup and 'touch_view_active' in ui

# The saved wet knob is MIN; audio gets its separate live wet snapshot.
assert 'v.fx_mod_live=ru_touch_level_percent' in bringup
assert 'if(page==RU_PAGE_REVERB&&v->fx_mod_active)' in ui
assert '"LIVE REVERB"' in ui
assert '*value=ru_touch_percent_q15(min);native_reverb=*value;' in bringup
assert 'ru_touch_profile_dirty|=' in bringup
assert 'ru_touch_apply_profile(&c,&touch_profile,expr_level)' in (native/'audio_target.c').read_text()

print('PASS V2.0.58: ADC DRIVE2/divider7, LIVE raw + FX wet display, two-way reverb MIN binding')
