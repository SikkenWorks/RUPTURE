#!/usr/bin/env python3
"""OTA LCD must move during CHECK without corrupting borrowed LZ4 scratch."""
from pathlib import Path
root=Path(__file__).resolve().parents[1]
target=(root/'vendor/etch-v1.88-source/firmware/fm1-native/ota_target.c').read_text()
core=(root/'vendor/etch-v1.88-source/firmware/fm1-native/ota.c').read_text()
ui=(root/'vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c').read_text()
html=(root/'WebInstaller.html').read_text()
assert 'ota_progress_part(0u,45u,i+n,len)' in core
assert 'ota_progress_part(45u,50u,i-32u,area_size-32u)' in core
assert 'ota_progress_part(0u,85u,i+n,stage_len)' in core
assert 'ota_progress_part(85u,14u,i+n,stage_len)' in core
assert 'if(ota_matrix_step!=1u && ota_matrix_step!=3u)return;' in target
assert 'lcd_fill(20u+old_width,123u,next_width-old_width,15u,0xffffu);' in target
assert 'ota_progress_paint_delta();' in target
assert 'ru_render(&state);ru_commit();' in target
assert 'if((uint32_t)(now-ota_matrix_last)>=100u)' in target
assert 'READING / VERIFYING' in ui and 'WRITING OTA LOADER' in ui
assert "if(name==='check')next=Math.min(19,Math.max(0,value)*.19)" in html
assert 'byteCoverage(pkg.image.length)' in html
assert 'Math.min(90,n)' not in html
assert "this.progress('check',100,'CHECK confirmed by FM1" in html
assert 'lcd_fill' not in core,'Transport core must stay platform-neutral'
assert 'fm1_enter_update(parm);' in target,'Trusted OTA loader handoff preserved'
print('PASS V2.0.58 OTA wiring: measured read/write, scratch-safe LCD, trusted loader unchanged')
