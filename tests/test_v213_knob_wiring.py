#!/usr/bin/env python3
"""Static integration checks for the main-loop PRESETS/EXPRESSION wiring."""
from pathlib import Path

native = Path(__file__).resolve().parents[1] / 'vendor/etch-v1.88-source/firmware/fm1-native'
main = (native / 'bringup.c').read_text()
audio = (native / 'audio_core.c').read_text()
ui = (native / 'rupture_ui.c').read_text()
store = (native / 'wave_store.c').read_text()
assign = (native / 'rupture_preset_assign.h').read_text()

assert '#define RU_PRESET_ASSIGN_EXPRESSION 94u' in assign
assert '{9,RU_PAGE_PLAY,4,"EXPRESSION LATCH"}' in assign
assert '{9,RU_PAGE_PLAY,5,"EXPRESSION RETURN"}' in assign
assert '#define RU_PRESET_ASSIGN_EXPRESSION_RETURN 95u' in assign
assert 'ru_preset_growl_assign=RU_PRESET_ASSIGN_EXPRESSION' in main
assert 'return ru_main_wave?ru_preset_growl_assign:ru_preset_assign;' in main
assert 'ru_settings_open=4u; /* Select CXS or GROWL first. */' in main
assert 'ru_settings_open==4u' in main
assert 'ru_growl_expression=(uint8_t)ui_bound' in main
assert 'ru_growl_expression_return_step' in main
assert 'ru_growl_expression_service();' in main
assert 'etch_store_preset_assign_pair_save(&native_store_io' in main
assert 'etch_store_preset_assign_pair_load(&native_store_io' in main
assert 'ru_preset_apply(preset);' in main.split('/* V2.0.7 FIX')[1].split('continue;')[0]
assert 'SEQ + PRESETS' in main or 'Hold SEQ + PRESETS' in main
assert 'ASSIGN GROWL PRESETS' in ui
assert 'PRESETS KNOB / MODE' in ui
assert 'v->settings_open==4u' in ui
assert 'if(ru_growl_expression_smooth<target)' in audio
assert 'ru_growl_apply_expression(&growl_params,ru_growl_expression_smooth);' in audio
assert 'RU_PRESET_ASSIGN_PAIR_MAGIC' in store
assert 'ru_preset_assign_pair_scan(io,source_bank' in store
assert 'ru_preset_assign_pair_scan(io,(uint32_t)bank' in store
assert 'ru_vol_compact' in store
print('PASS V2.0.14: CXS/GROWL mapping, LATCH/RETURN main-loop service')
