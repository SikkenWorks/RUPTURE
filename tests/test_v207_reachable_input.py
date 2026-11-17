#!/usr/bin/env python3
"""Guard the exact V2.0.6 cause: PRESETS was after RUPTURE's continue."""
from pathlib import Path

src=(Path(__file__).resolve().parents[1]/
    'vendor/etch-v1.88-source/firmware/fm1-native/bringup.c').read_text()
start=src.index('if(!ru_autosave_armed && native_store_status==ETCH_STORE_OK')
end=src.index('   continue;\n#endif',start)
real_runtime=src[start:end]
assert 'preset,buttons,ru_page,ru_settings_open' in real_runtime
assert 'ru_preset_input_action(' in real_runtime
assert 'ru_preset_apply(preset);' in real_runtime
assert 'ru_seq_rotate(&native_pattern,staged,native_seq_length,preset)' in real_runtime
assert '||preset||' in real_runtime, 'PRESETS must wake the screensaver'
assert real_runtime.index('ru_preset_apply(preset);') < real_runtime.index('ru_dam_changed')
assert 'ru_preset_popup_until=etch_ms+1550u' in src
assert 'int32_t preset=fm1_enc_take(6)' in src

legacy=src[end:]
assert '#ifndef RUPTURE_BUILD' in legacy and 'ru_preset_apply(preset);' in legacy
assert legacy.index('#ifndef RUPTURE_BUILD') < legacy.index('ru_preset_apply(preset);')
print('PASS V2.0.7 PRESETS is reachable BEFORE RUPTURE continue, legacy ETCH separated')
