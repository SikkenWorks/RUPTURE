#!/usr/bin/env python3
"""Audit real CPU1 full-FX ownership while protecting stable PCM code."""
from pathlib import Path
import hashlib
import json

root=Path(__file__).resolve().parents[1]
native=root/'vendor/etch-v1.88-source/firmware/fm1-native'
stable=(native/'audio_core.c').read_bytes()
experimental=(native/'audio_core_core2.c').read_text()
worker=(native/'rupture_cpu1_lab.h').read_text()
bringup=(native/'bringup.c').read_text()
build=(native/'build.py').read_text()
fx=(native/'native_fx.c').read_bytes()
cxs=(native/'native_cxs_q15.c').read_bytes()

sha=lambda b:hashlib.sha256(b).hexdigest()
assert sha(stable)=='0e5588d24d41f69be5e3065acf4218795a4b55916f448a1b595d984fde7eb235'
gain_after='''  /* PCM from CPU1 and the preview mixer is already signed 16-bit. At
   * Q15 gain <= 32768 the product/division cannot leave that range,
   * so two saturation checks do no work. Retain the original exact
   * clamp path for malformed/out-of-range gains (up to UINT16_MAX). */
  const int32_t post=(int32_t)v->post_gain;
  if(post<=32768){
   l=(l*post)/32768;
   r=(r*post)/32768;
  }else{
   l=clamp15((l*post)/32768);
   r=clamp15((r*post)/32768);
  }
'''
gain_before='''  l=clamp15((l*(int32_t)v->post_gain)/32768);
  r=clamp15((r*(int32_t)v->post_gain)/32768);
'''
assert experimental.count(gain_after)==1
original_gain=experimental.replace(gain_after,gain_before,1)
assert sha(original_gain.encode())=='f77eab6c8c6200c27be25c51f90a44d04c06fda533d6e3aa46999c3e3d1594f4'
fastpath='''  /* Most blocks have metronome OFF (click=0). l/r are already clamped:
   * skip two more saturations then, preserving the exact 16-bit PCM.
   * Test click per FRAME, not per block: its enabled state is volatile. */
  if(v->click){
   out[f*2]=(int16_t)clamp15(l+v->click);
   out[f*2+1]=(int16_t)clamp15(r+v->click);
  }else{
   out[f*2]=(int16_t)l;
   out[f*2+1]=(int16_t)r;
  }
'''
legacy='''  out[f*2]=(int16_t)clamp15(l+v->click);
  out[f*2+1]=(int16_t)clamp15(r+v->click);
'''
assert experimental.count(fastpath)==1
assert sha(original_gain.replace(fastpath,legacy,1).encode())=='34b7f6609db9b985403790b7176d7bb915381505f05d40bb961d0c209d6a4464', 'Only the two final fast paths may differ from previous CORE2 DSP'
assert sha(fx)=='d7a7b55f99011d92a98f8ce8f20a6d2ec3374fc8f67fdcac48c70a05d248412a'
assert sha(cxs)=='86f4a39ad9bdbd6eabb79a34d9453e0fb0e150936667806e94eb2f00b57ca18f'
assert 'ru_mono_panel_reconcile(s,keys)' in stable.decode()
assert 'ru_mono_panel_reconcile(s,keys)' in experimental
assert "('audio_core_core2.c' if args.dual_core_fx_live_lab else 'audio_core.c')" in build
assert '-DETCH_CPU1_FX_LIVE_LAB=' in build
for expected in (
 'uint32_t ru_fx_cpu1_run(',
 'rupture_fx_process_ordered_fast(ru_fx_c2.state',
 'etch_fx_process_stereo_masked(ru_fx_c2.state',
 'if(f==63u)ru_cpu1_fx_send(0u,ETCH_NATIVE_FRAMES);',
 'else if(f==127u)ru_cpu1_fx_publish(ETCH_NATIVE_FRAMES);',
 'ru_cpu1_fx_collect();',
 'for(uint32_t f=finished;f<ETCH_NATIVE_FRAMES;f++)ru_fx_c2_process_one(f);',
 'ru_fx_c2.settings[f>>5]=locked.fx_settings;',
 'ru_fx_c2_finish(out,0u,ETCH_NATIVE_FRAMES);',
):
 assert expected in experimental,expected
assert 'ru_fx_cpu1_run' in worker
assert 'ru_cpu1_retire(corrupt?RU_C1_BAD_OUTPUT:RU_C1_TIMEOUT)' in worker
assert 'ru_fx_probe_finished(ru_c1_good,ru_c1_fir_good)' in worker
assert 'ru_fx_lane_reply(seq,done,ru_c1_fx_box.bad' in worker
assert 'ru_fx_safe_progress(ru_c1_fx_box.begin,end,ru_c1_fx_box.progress)' in worker
assert 'ru_c1_good>=128u&&ru_c1_fir_good>=1u' in worker
assert 'ru_c1_fx_max_wait_ticks' in worker
assert 'label("FX JOBS",10,218' in bringup
assert 'label("FX US AVG",10,203' in bringup
assert 'number(ru_c1_fx_avg_wait_ticks/24u,106,203)' in bringup
assert 'label("MAX",151,203' in bringup
assert 'label("FX TIMEOUT",10,222' in bringup
assert 'label("FB",164,218,ru_c1_fx_fallbacks' in bringup
assert 'return 0;' in worker.split('static int ru_cpu1_uac_live_ready(void)',1)[1].split('#else',1)[0]

stable_options=root/'vendor/etch-v1.88-source/build/fm1-native-storage-updater-uac48-p05/native-build-options.json'
core2_options=root/'vendor/etch-v1.88-source/build/fm1-native-storage-updater-uac48-p05-c1fx/native-build-options.json'
if stable_options.is_file() and core2_options.is_file():
 stable_info=json.loads(stable_options.read_text())
 core2_info=json.loads(core2_options.read_text())
 assert not stable_info['cpu1_full_fx_live_lab_enabled']
 assert core2_info['cpu1_full_fx_live_lab_enabled']
 assert core2_info['cpu1_fx_chunk_frames']==64
 assert core2_info['cpu1_fx_stream_mailboxes_per_block']==1
 assert core2_info['cpu1_fx_stream_windows']==2
 assert not core2_info['cpu1_live_usb_fir_lab_enabled']
 assert stable_info['app_sha256']!=core2_info['app_sha256']

print('PASS V2.0.58: full FX worker on CPU1; exact CXS/FX sources; identical note guards on both cores')
