from pathlib import Path
r=Path(__file__).resolve().parents[1]
n=r/'vendor/etch-v1.88-source/firmware/fm1-native'
b=(n/'bringup.c').read_text();ui=(n/'rupture_ui.c').read_text()
assert 'if(native_store_status==ETCH_STORE_OK)native_project_browser_refresh();' in b
assert 'ru_legacy_empty_seq' in b and 'native_project_cfg.patterns.lengths[slot]!=0u)return 1u;' in b
assert 'ru_store_legacy_sequence_name' in (n/'wave_store.c').read_text()
assert 'RU_PAGE_BUG_REPORT' in ui and 'SCAN TO REPORT A BUG' in ui
assert 'ru_bug_qr_white' in ui and 'ru_bug_qr_black' not in ui
assert 'ru_land_horizon' in ui and 'ru_fault_center' in ui
assert 'ru_saver_frame<63u' in b
assert 'RU_PAGE_BUG_REPORT' in b and 'ru_selected+select,0,4' in b
assert 'ru_center(num,195,y+12,1)' in ui and 'ru_center(v->auto_save?"ON":"OFF",195,y+12,1)' in ui
assert not any(x in (r/'README.md').read_text() for x in ('# V1.29', '# V1.30', '# V1.31', '## Changelog'))
print('PASS V1.30 native input + storage source contract')
