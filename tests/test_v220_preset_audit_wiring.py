from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
SRC=ROOT/'vendor/etch-v1.88-source/firmware/fm1-native'
bringup=(SRC/'bringup.c').read_text()
audio=(SRC/'audio_target.c').read_text()
ui=(SRC/'rupture_ui.c').read_text()

load=bringup.split('static void native_patch_load(uint8_t pattern_prepared){',1)[1].split('static void native_strings_preview',1)[0]
save=bringup.split('static void native_patch_save(uint8_t muted){',1)[1].split('static void native_patch_change(uint8_t next){',1)[0]
assert 'native_preset_profile_reset();' in load
assert 'ru_preset_audit_loaded(native_patch_slot);' in load
assert load.index('native_parameter_publish();')<load.index('ru_preset_audit_loaded(native_patch_slot);')
assert 'ru_preset_audit_saved(target);' in save
assert save.index('if(native_store_status!=ETCH_STORE_OK)')<save.index('ru_preset_audit_saved(target);')
assert 'ru_preset_audit_saved(native_patch_slot)' in bringup
assert 'ru_preset_saved_mask=0u' in bringup
assert 'native_preset_render_peak' in audio and 'native_preset_wall_peak' in audio
assert 'native_preset_late_base=native_late' in audio
assert 'native_preset_guard_base=native_dma_blocked' in audio
assert 'native_preset_ramp_requested' in audio
assert 'ru_diag_view+1u)%5u' in bringup
assert 'READ ONLY / PRESET AUDIT' in ui and 'SAVE / LOAD' in ui
assert 'V2.0.39' in ui
assert '#include "rupture_preset_audit.h"' in bringup
print('PASS V2.0.20 source wiring: profile on preset handoff / verified-save fingerprint / diagnostic page')
