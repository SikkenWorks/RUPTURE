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
 'audio_core.c':'6499bf5d022ac8e61457d52a7400f109794c5d5ae790d6ec3899972bcd1062c9',
 'native_fx.c':'d7a7b55f99011d92a98f8ce8f20a6d2ec3374fc8f67fdcac48c70a05d248412a',
 'usb.c':'7992a47f973b73844477af454c6401063415964709645a68cf563aac5cd62d34',
}.items():
 actual=sha((src/name).read_bytes())
 assert actual==expected,(name,actual,expected)

# Reverse only V239's optional oscilloscope guard first. V238's ALNK0 file
# must be restored byte-for-byte, including the earlier read-only counters.
audio=(src/'audio_target.c').read_text()
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
