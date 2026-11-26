#!/usr/bin/env python3
"""Current storage/audio candidate checks; historical release contracts stay historical."""
from pathlib import Path
import os, subprocess, tempfile
R=Path(__file__).resolve().parents[1]
N=R/'vendor/etch-v1.88-source/firmware/fm1-native'
os.chdir(R)
subprocess.run(['python3','tests/test_v260_storage_audio.py'],check=True,
 env={**os.environ,'ASAN_OPTIONS':os.environ.get('ASAN_OPTIONS','detect_leaks=0')})
fx=['native_fx.c','rupture_eq2.c']
sound=fx+['native_cxs_q15.c','wave_editor.c','native_performance.c','native_wave_fx.c']
store=['wave_store.c','native_project_codec.c','native_pattern_bank.c','native_performance.c']+fx
cases=[
 ('tests/test_v210_zero_wet_slots.c',fx,[]),
 ('tests/test_v138_all_filters.c',[],[]),
 ('tests/test_v210_sub_eq_reverb_stress.c',fx+['native_cxs_q15.c'],[]),
 ('tests/test_v211_growl_eq_reverb_stress.c',fx+['native_cxs_q15.c'],[]),
 ('tests/test_v213_expression_eq_reverb_stress.c',fx+['native_cxs_q15.c'],[]),
 ('tests/test_v217_growl_sub_distortion_save.c',store,[]),
 ('tests/test_v219_preset_load_rt.c',[],[]),
 ('tests/test_v238_note_dma_meter.c',[],[]),
 ('tests/test_v239_note_scope_guard.c',[],[]),
 ('tests/test_v226_fx_combinations.c',fx+['native_cxs_q15.c'],[]),
 ('tests/test_v258_note_release_runtime.c',['audio_core.c']+sound,[]),
 ('tests/test_v258_note_release_runtime.c',['audio_core_core2.c']+sound,['-DETCH_CPU1_FX_LIVE_LAB=1']),
 ('tests/test_v258_core2_fx_pcm.c',['audio_core_core2.c']+sound,['-DETCH_CPU1_FX_LIVE_LAB=1']),
 ('tests/test_v258_core2_fx_stream.c',[],[]),
 ('tests/test_v258_core2_fx_guard.c',[],[]),
 ('tests/test_v258_core2_pressure_guard.c',[],[]),
 ('tests/test_v259_pressure_and_fx_mix.c',[],[]),
 ('tests/test_v259_mosfet_pcm.c',fx,[]),
 ('tests/test_v258_profile_persistence.c',store,[]),
 ('tests/test_v258_touch_persistence.c',store,[]),
 ('tests/test_v117_flash_fx.c',store,[]),
 ('tests/test_v128_storage.c',store,[]),
 ('tests/test_v231_usb_preference.c',store,[]),
 ('tests/test_v231_usb_descriptor.c',[],[]),
 ('tests/test_v230_uac_packed_pcm.c',[],[]),
 ('tests/test_v234_live_uac_fir.c',[],[]),
 ('vendor/etch-v1.88-source/tests/test_native_startup_pop.c',[],[]),
 ('vendor/etch-v1.88-source/tests/test_native_preset_ramp_v108.c',[],[]),
]
with tempfile.TemporaryDirectory(prefix='rupture-v260-regression-') as tmp:
 for i,(test,src,flags) in enumerate(cases):
  exe=str(Path(tmp)/str(i))
  cmd=[os.environ.get('CC','gcc'),'-std=c11','-O2','-w',
    '-fsanitize=undefined','-fno-sanitize-recover=all','-DRUPTURE_BUILD',
    '-DETCH_P05_MULTI=1','-DETCH_STORE_OWNED_ENABLED=1','-I'+str(N),
    *flags,test,*[str(N/s) for s in src],'-lm','-o',exe]
  subprocess.run(cmd,check=True)
  subprocess.run([exe],check=True)
print('PASS all',len(cases),'PCM, MIDI, CORE2, storage, USB and startup regression programs')
