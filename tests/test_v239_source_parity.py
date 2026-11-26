#!/usr/bin/env python3
"""V239 changes only the OPTIONAL scope path, never audible CXS/FX PCM."""
from pathlib import Path
import hashlib

root=Path(__file__).resolve().parents[1]
src=root/'vendor/etch-v1.88-source/firmware/fm1-native'
sha=lambda data:hashlib.sha256(data).hexdigest()

# These complete native modules, not just selected functions, must remain
# byte-identical to the audited V237 firmware.
for name,expected in {
 'native_cxs_q15.c':'86f4a39ad9bdbd6eabb79a34d9453e0fb0e150936667806e94eb2f00b57ca18f',
 'audio_core.c':'0e5588d24d41f69be5e3065acf4218795a4b55916f448a1b595d984fde7eb235',
 'native_fx.c':'d7a7b55f99011d92a98f8ce8f20a6d2ec3374fc8f67fdcac48c70a05d248412a',
 'usb.c':'7992a47f973b73844477af454c6401063415964709645a68cf563aac5cd62d34',
}.items():
 actual=sha((src/name).read_bytes())
 assert actual==expected,(name,actual,expected)

# Reverse only V239's optional oscilloscope guard first. V238's ALNK0 file
# must be restored byte-for-byte, including the earlier read-only counters.
audio=(src/'audio_target.c').read_text()
# V258 adds a bounded, optional MIDI observer. Rewind it in full before
# checking the audited historical ALNK0 hash, including its MIDI releases.
new_state='''#ifdef RUPTURE_BUILD
/* Eight poly-aftertouch tags and 16 channel-pressure values; bounded state
 * only at MIDI event/block boundaries, never inside CXS or CORE2 DSP. */
static struct ru_midi_expression ru_midi_expr;
#endif
'''
assert audio.count('#include "rupture_midi_expression.h"\n')==1
assert audio.count(new_state)==1
audio=audio.replace('#include "rupture_midi_expression.h"\n','',1).replace(new_state,'',1)
obs_overflow='''  ru_midi_expr_reset(&ru_midi_expr);
'''
assert audio.count(obs_overflow)==2
audio=audio.replace('''#ifdef RUPTURE_BUILD
  if(native_midi_panic_seen<UINT32_MAX)++native_midi_panic_seen;
  ru_midi_expr_reset(&ru_midi_expr);
#endif
''','''#ifdef RUPTURE_BUILD
  if(native_midi_panic_seen<UINT32_MAX)++native_midi_panic_seen;
#endif
''',1)
audio=audio.replace('''#ifdef RUPTURE_BUILD
   ru_midi_expr_reset(&ru_midi_expr);
#endif
''','',1)
audio=audio.replace('''#ifdef RUPTURE_BUILD
   ru_midi_expr_event(&ru_midi_expr,status,note,velocity);
#endif
''','',1)
obs_start='''#ifdef RUPTURE_BUILD
 ru_midi_expr_reset(&ru_midi_expr);
 ru_touch_midi_held=0u;ru_touch_midi_source=0u;
 ru_touch_midi_raw=0u;ru_touch_audio_level=0u;
#endif
'''
assert audio.count(obs_start)==1
audio=audio.replace(obs_start,'',1)
new_declaration='''#ifdef RUPTURE_BUILD
  /* Per-voice expression changes ONLY the immutable ALNK block snapshot.
   * Neither CXS oscillator math nor the two CPU cores' FX code changes.
   * OFF returns before touching controls; the realtime path owns no RNG. */
  const struct ru_touch_profile touch_profile=ru_touch_profiles[c.main_wave?1u:0u];
#endif
'''
new_observer='''#ifdef RUPTURE_BUILD
  /* MIDI Note On has now reached the ORIGINAL bounded mono stack. Read only
   * the actual winner, so stacked/released pitches, scale locks and sustain
   * keep their original ownership. Do not invent or delay a Note OFF. */
  uint8_t expr_level=ru_touch_live_motion;
  uint8_t expr_source=RU_MIDI_EXPR_NONE,expr_raw=0u;
  ru_touch_midi_held=ru_midi_expr_held(&native_audio);
  if(ru_touch_midi_held &&
     (touch_profile.mode==RU_TOUCH_MODE_PRESS||touch_profile.mode==RU_TOUCH_MODE_VELOCITY)){
   expr_level=ru_midi_expr_level(&ru_midi_expr,&native_audio,&touch_profile,
      &expr_source,&expr_raw);
  }else if(touch_profile.mode==RU_TOUCH_MODE_VELOCITY){
   /* Internal panel/sequence voices already have fixed per-note velocity.
    * The FM1 panel's physical force sensor remains PRESS, not MIDI velocity. */
   const struct etch_native_voice *v=&native_audio.voice[0];
   expr_level=(v->active&&v->held)?
      ru_midi_velocity_level(v->velocity,touch_profile.sensitivity):0u;
  }
  ru_touch_midi_source=expr_source;ru_touch_midi_raw=expr_raw;
  ru_touch_audio_level=expr_level;
  if(touch_profile.target)ru_touch_apply_profile(&c,&touch_profile,expr_level);
#endif
'''
assert audio.count(new_declaration)==1 and audio.count(new_observer)==1
audio=audio.replace(new_observer,'',1)
# Restore original V249 control-only touch block in its old position.
v258_touch='''#ifdef RUPTURE_BUILD
  /* Per-voice expression changes ONLY the immutable ALNK block snapshot.
   * Neither CXS oscillator math nor the two CPU cores' FX code changes.
   * OFF returns before touching controls; the realtime path owns no RNG. */
  const struct ru_touch_profile touch_profile=ru_touch_profiles[c.main_wave?1u:0u];
  if(touch_profile.target)
   ru_touch_apply_profile(&c,&touch_profile,ru_touch_live_motion);
#endif
'''
assert audio.count(new_declaration)==1
audio=audio.replace(new_declaration,v258_touch,1)
assert audio.count(v258_touch)==1
audio=audio.replace(v258_touch,'',1)
# First reverse the V245 CORE2-DMA failover. This must recover V244's
# complete original audio ISR, proving there were no other audio changes.
v258_late='''#if ETCH_CPU1_FX_LIVE_LAB
 /* An actual CORE2 FX lane in this missed DMA block is evidence that the
  * experimental intercore route did not meet the realtime deadline.
  * All FX lanes have completed or retired before reaching this callback.
  * Switch to the bit-exact CPU0 path for subsequent blocks instead of
  * repeatedly risking an audio-engine stop on this same CORE2 workload. */
 if(ru_core2_pressure_should_retire(ru_c1_online,
      ru_fx_probe_finished(ru_c1_good,ru_c1_fir_good),
      core2_jobs_before,ru_c1_fx_commits)){
  ru_cpu1_retire(RU_C1_PRESSURE);
 }
#else
 (void)core2_jobs_before;
#endif
'''
v258_capture='''  /* Compare completed jobs only within THIS block, including MIDI, USB,
   * render and publish. Earlier FX activity cannot trip CORE2 on an
   * unrelated LCD/USB-only deadline miss. */
#if ETCH_CPU1_FX_LIVE_LAB
  const uint32_t core2_jobs_before=ru_c1_fx_commits;
#else
  const uint32_t core2_jobs_before=0u;
#endif
'''
assert audio.count(v258_late)==1 and audio.count(v258_capture)==1
audio=audio.replace(v258_late,'',1).replace(v258_capture,'',1)
assert audio.count('#include "rupture_core2_pressure_guard.h"\n')==1
audio=audio.replace('#include "rupture_core2_pressure_guard.h"\n','',1)
assert audio.count('native_budget_late(core2_jobs_before);')==2
audio=audio.replace('native_budget_late(core2_jobs_before);','native_budget_late();')
assert audio.count('static void native_budget_late(uint32_t core2_jobs_before){')==1
audio=audio.replace('static void native_budget_late(uint32_t core2_jobs_before){',
 'static void native_budget_late(void){',1)
assert sha(audio.encode())=='7866683acc8aa4a20db8e410a6feb8f8827f6053507a1cdd93c81b4fef4a2ee9'
# V244 adds strictly read-only MIDI-event/overflow counters. Rewind only
# those counters before verifying the original V243/V239/V238 ISR bodies.
v258_counters='''/* Diagnose intermittent MIDI release faults without printf/heap in ALNK0.
 * Counters remain cumulative across an audio-engine restart. */
#ifdef RUPTURE_BUILD
static volatile uint32_t native_midi_on_seen,native_midi_off_seen;
static volatile uint32_t native_midi_panic_seen;
#endif
'''
v258_overflow=''' if(midi_in_overflow){
#ifdef RUPTURE_BUILD
  if(native_midi_panic_seen<UINT32_MAX)++native_midi_panic_seen;
#endif
  etch_native_midi_panic(&native_audio);mi_r=end;RING_PUBLISH();midi_in_overflow=0;native_midi_backlog=0u;return;
 }
'''
v243_overflow=' if(midi_in_overflow){etch_native_midi_panic(&native_audio);mi_r=end;RING_PUBLISH();midi_in_overflow=0;native_midi_backlog=0u;return;}\n'
v258_edges='''#ifdef RUPTURE_BUILD
   if((status&0xf0u)==0x80u || ((status&0xf0u)==0x90u&&!velocity)){
    if(native_midi_off_seen<UINT32_MAX)++native_midi_off_seen;
   }else if((status&0xf0u)==0x90u&&velocity){
    if(native_midi_on_seen<UINT32_MAX)++native_midi_on_seen;
   }
#endif
'''
for old,new in ((v258_counters,''),(v258_overflow,v243_overflow),(v258_edges,'')):
 assert audio.count(old)==1,'V244 MIDI telemetry insertion must be exact'
 audio=audio.replace(old,new,1)
# V243's MIDI learn change is strictly confined to NOTE ON capture.
# Rewind its specific text before applying historical V239/V238 checks.
v243_insert='''   /* V2.0.43: do NOT swallow MIDI NOTE OFF during group-key capture.
    * Previously this silently discarded the release of a note that was
    * already sounding when capture began, leaving a latched mono voice.
    * A capture-only key attack was never voiced, so its later OFF is safe. */
'''
assert audio.count(v243_insert)==1
audio=audio.replace(v243_insert,
 '   if(native_group_capture_active && ((status&0xf0u)==0x80u || (status&0xf0u)==0x90u))continue;\n')
assert audio.count('if(ru_midi_capture_attack(native_group_capture_active,status,velocity)&&note<128u)')==1
audio=audio.replace('if(ru_midi_capture_attack(native_group_capture_active,status,velocity)&&note<128u)',
 'if(native_group_capture_active && (status&0xf0u)==0x90u && velocity && note<128u)')
assert audio.count('#include "rupture_note_release_guard.h"\n')==1
audio=audio.replace('#include "rupture_note_release_guard.h"\n','')
scope_only=[
 '#include "rupture_note_scope_guard.h"\n',
 '''/* Owned by ALNK0: protects ONLY oscilloscope samples, never PCM or MIDI. */
static ru_note_scope_guard_t native_note_scope_guard;
''',
 '''#ifdef RUPTURE_BUILD
  /* Skip mono scope averaging/capture around note on/off with 3+ FX, or
   * at <=20% slack for ANY chain, including MOSFET+REVERB. Never run an FX count in the
   * common no-pressure/no-edge case. All 128 PCM frames still publish. */
  const uint32_t scope_budget=ETCH_NATIVE_FRAMES*544u;
  const uint32_t scope_slack=native_last_dma_slack;
  const uint8_t inspect_fx=(uint8_t)(nd_edges||
      native_note_scope_guard.quiet_halves);
  const uint8_t scope_fx=inspect_fx?
      ru_note_fx_active_count(&c.fx_chain,c.fx_chain_active):0u;
  const uint8_t normal_scope=prepare_scope;
  prepare_scope=ru_note_scope_capture_allow(&native_note_scope_guard,
      prepare_scope,nd_edges,scope_fx,scope_slack,scope_budget);
  if(normal_scope&&!prepare_scope&&native_scope_deferred<UINT32_MAX)
   ++native_scope_deferred;
#endif
''',
 ' native_note_scope_guard.quiet_halves=0u;\n',
]
for snippet in scope_only:
 assert audio.count(snippet)==1,'V239 scope-only insertion differs from audited change'
 audio=audio.replace(snippet,'',1)
assert sha(audio.encode())=='27b193376910de6102a804d37c37c421d27cff29c14415255a91fe49cfdebfaa'

# Reverse V238 instrumentation next, to the original V237 ISR hash.
added=[
 '#include "rupture_note_dma_meter.h"\n',
 '''/* Only the ALNK0 interrupt writes these counters; foreground reads 32-bit
 * snapshots. They are intentionally cumulative across audio restarts. */
static volatile ru_note_dma_meter_t native_note_dma;
''',
 '''#ifdef RUPTURE_BUILD
  const uint32_t nd_old_keys=native_audio.keys;
  const uint32_t nd_old_serial=native_audio.alloc_serial;
  const uint8_t nd_old_stack=native_audio.mono_count;
  const uint8_t nd_old_held=native_audio.voice[0].held;
#endif
''',
 '''#ifdef RUPTURE_BUILD
  const uint8_t nd_edges=native_output_enabled?
    ru_note_dma_edge(nd_old_keys,native_audio.keys,nd_old_serial,
      native_audio.alloc_serial,nd_old_stack,native_audio.mono_count,
      nd_old_held,native_audio.voice[0].held):0u;
#endif
''',
 '''#ifdef RUPTURE_BUILD
   ru_note_dma_record(&native_note_dma,nd_edges,render_elapsed,
                      (uint32_t)(fm1_ticks()-start),1u);
#endif
''',
 '''#ifdef RUPTURE_BUILD
  ru_note_dma_record(&native_note_dma,nd_edges,render_elapsed,
                     measure.wall_ticks,measure.late);
#endif
''',
]
for snippet in added:
 assert audio.count(snippet)==1,'ISR instrumentation differs from audited insertion'
 audio=audio.replace(snippet,'',1)
assert sha(audio.encode())=='3813d7ac1b2d62c4c57a8a2221cf4cb7224bacb4ae79104995dc227a87dfabf6'
print('PASS V2.0.43: V239/V238 ISR history still matches after reversing targeted MIDI release fix')
