from pathlib import Path
import re,hashlib
r=Path(__file__).resolve().parent.parent
n=r/'vendor/etch-v1.88-source/firmware/fm1-native'
b=(n/'bringup.c').read_text();ui=(n/'rupture_ui.c').read_text();store=(n/'wave_store.c').read_text()
assert 'ru_autosave_enabled&&rupture_autosave_due' in b
assert 'ru_autosave_enabled=0u;' in b
assert 'else ru_adjust(ru_focus,alg);' not in b
assert 'ru_seq_extend_length' in b and 'ru_name_change(alg)' in b
assert 'ru_sub_blend_amount[z]+alg' in b and 'ru_help_scroll+alg' in b
assert 'ru_saver_delay_seconds[ru_saver_delay_index]*1000u' in b
assert 'native_seq_run||native_seq_recording' in b
assert 'int visible=v->seq_length<16u?' in ui
assert 'ru_crack_saver' in ui and 'if(v->saver_active)' in ui
assert 'RU_SETTINGS_MAGIC' in store and 'ru_vol_transfer_preferences' in store
assert 'ru_vol_scan(io,source_bank,&vol,&autosave,&saver,&seconds,&boot,0,&crt,&done,0)' in store
assert 'ru_preset_assign_pair_scan(io,source_bank,&mapping,&growl,&mapping_present)' in store
readme=(r/'README.md').read_text()
assert not re.search(r'^#{1,3}\s*(?:V\d+|CHANGELOG|RELEASE HISTORY)',readme,re.I|re.M)
assert not re.search(r'^\s*[-*]\s*(?:V\d+\.\d+:)',readme,re.M)
licensing=(r/'licenses/FELUCCA-LICENSING.txt').read_text()
assert 'SIKKENWORKS' in readme.upper() and 'Leo Kuroshita' in licensing and 'GPL-3.0-only' in licensing
assert 'autosave' in readme.lower() and 'screensaver' in readme.lower()
print('PASS V1.27 native source contract, global settings preserved across bank rewrites, README history separation')
