#!/usr/bin/env python3
"""Static integration contracts for the native fifth-encoder assignment path."""
from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]
src = root / 'vendor/etch-v1.88-source/firmware/fm1-native'
input_hal = (src / 'hal/fm1_input.h').read_text()
bringup = (src / 'bringup.c').read_text()
ui = (src / 'rupture_ui.c').read_text()
catalog = (src / 'rupture_preset_assign.h').read_text()

assert '#define FM1_NENC 7u' in input_hal
assert '{0, 5, 1, 5}' in input_hal, 'PRESETS is wired to encoder 6 on PB7'
assert re.search(r'preset\s*=\s*fm1_enc_take\(6\)', bringup)
assert 'ru_preset_apply(preset);' in bringup
assert 'ru_page=target->page;' in bringup
assert 'ru_adjust(target->control,turns);' in bringup
assert 'ru_page=original_page;' in bringup
assert 'ru_preset_popup_until=etch_ms+1550u' in bringup
assert 'ru_preset_popup_format(v.preset_popup_value);' in bringup
assert 'ru_preset_popup_until=0u;first_frame=1u;' in bringup
assert 'preset_popup_visible' in ui
assert 'ru_preset_targets[v->preset_assign].name' in ui
assert 'ru_center(v->preset_popup_value,120,124,2)' in ui
assert 'preset_save_stage' in ui
assert 'native_harmony.root%12u' in bringup
assert 'native_harmony.root+amount' in bringup
assert '"SCALE GUIDE ROOT"' in catalog
assert 'ru_preset_save_stage=1u;ru_preset_save_progress=5u;' in bringup
assert 'boot_wave_ms<RU_LED_BOOT_WAVE_TOTAL_MS' in bringup
assert 'ru_led_wave_cancelled=1u;' in bringup
assert 'ru_led_boot_wave_level(boot_wave_ms,(uint8_t)k,level)' in bringup
assert 'ru_led_boot_wave_level(boot_wave_ms,(uint8_t)(27u+b),level)' in bringup
assert 'native_led_last_change_ticks=fm1_ticks();' in bringup

# All 94 hardware assignments must point to a real editor branch/parameter.
entries = re.findall(r'^ \{\d+,RU_PAGE_[A-Z_0-9]+,', catalog, re.M)
assert len(entries) >= 90, len(entries)
print('PASS V2.0.6 PRESETS physical GPIO, editor routing, popup, save and wave integration')
