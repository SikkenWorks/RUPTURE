#!/usr/bin/env python3
"""Verify EVERY save path captures PJT35, independently of project UI."""
from pathlib import Path

src = Path(__file__).resolve().parents[1] / 'vendor/etch-v1.88-source/firmware/fm1-native'
save = (src / 'wave_store_target.c').read_text()
bringup = (src / 'bringup.c').read_text()
capture = save.split('static void native_project_capture(void){', 1)[1].split('static void native_library_commit(', 1)[0]
assert 'ru_fx_store_to_project();' in capture
assert 'native_project_cfg.source_version=35u;' in capture
assert 'native_project_cfg.source_version=34;' not in capture
assert 'native_project_capture();' in save.split('static void native_library_commit(', 1)[1]
assert 'native_library_commit(muted);' in bringup.split('static void native_patch_save(',1)[1].split('static void native_patch_change(',1)[0]
assert 'native_library_commit(muted);' in bringup.split('static void native_project_browser_save(',1)[1].split('static void native_project_browser_delete(',1)[0]
assert 'ru_fx_store_to_project();' in bringup
assert 'native_project_cfg.source_version=35u;' in bringup
print('PASS V2.0.17: PRESET / PROJECT / autosave all capture PJT35 and SUB blend')
