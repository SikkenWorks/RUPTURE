#!/usr/bin/env python3
"""Verify preset-only wavetable work cannot preempt active RUPTURE audio."""
from pathlib import Path

native=Path(__file__).resolve().parents[1]/'vendor/etch-v1.88-source/firmware/fm1-native'
target=(native/'audio_target.c').read_text()
bringup=(native/'bringup.c').read_text()
ramp=(native/'native_preset_ramp.h').read_text()

assert 'static uint8_t native_patch_wave_pending;' in target
assert 'if(pattern_prepared)native_patch_wave_pending=1u;' in bringup
assert '!(native_patch_wave_pending && native_blend_audio_busy())' in bringup
assert 'if(!native_editor.dirty)native_patch_wave_pending=0u;' in bringup
assert 'etch_draw_compile_due(etch_ms,last_wave_compile_ms' in bringup
assert 'native_compile_edit();last_wave_compile_ms=etch_ms;' in bringup
assert 'if(pattern_prepared)native_preset_ramp_requested=1u;' in bringup
assert 'etch_preset_ramp_begin(&native_preset_ramp);' in target
assert 'etch_preset_ramp_q8(r->hold_l*' in ramp
assert 'etch_preset_ramp_q8(r->hold_r*' in ramp
assert '*l=etch_preset_ramp_q8' in ramp
assert 'ru_sub_rt_tick(&ru_sub_rt_state[0]' in (native/'audio_core.c').read_text()
print('PASS V2.0.19: active-note preset wave compilation deferred; ramp and SUB routing retained')
