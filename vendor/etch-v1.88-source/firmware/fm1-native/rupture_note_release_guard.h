/* SPDX-License-Identifier: GPL-3.0-only
 * RUPTURE V2.0.43: tiny note-release ownership checks, once per PCM block.
 * Never time out a genuinely held key or truncate a sustained MIDI note.
 */
#ifndef RUPTURE_NOTE_RELEASE_GUARD_H
#define RUPTURE_NOTE_RELEASE_GUARD_H
#include <stdint.h>

/* Group-key MIDI learn must NOT eat Note OFFs from previously sounding
 * notes. In USB MIDI, 0x90 velocity=0 is also an explicit Note OFF.
 * Only capture/suppress real key attacks; all releases reach the synth. */
static inline uint8_t ru_midi_capture_attack(uint8_t capture,uint8_t status,
                                             uint8_t velocity){
 return (uint8_t)(capture && (status&0xf0u)==0x90u && velocity!=0u);
}

/* A physical FM1 key is a LEVEL, not a number of stacked MIDI events.
 * A missed/reordered edge or memory corruption must not leave an orphan
 * panel note in the monophonic key stack. MIDI/SEQ notes are not touched:
 * they can legally outlast a physical key and support sustain.
 * The caller resumes the winning mono voice only if the stack changed. */
static inline uint8_t ru_mono_panel_reconcile(struct etch_native_audio *s,
                                               uint32_t keys){
 if(!s)return 0u;
 if(s->mono_count>ETCH_MONO_STACK){
  s->mono_count=0u;
  s->voice[0].held=s->voice[0].down=0u;
  s->voice[0].gate_left=0u;
  return 1u;
 }
 uint8_t out=0u,changed=0u;
 for(uint8_t i=0u;i<s->mono_count;i++){
  struct etch_mono_held m=s->mono_stack[i];
  if(m.id<27u && !(keys&(1u<<m.id))){changed=1u;continue;}
  /* A contact has at most one physical down state. Suppress corrupted
   * duplicates, preserving the earliest remaining priority ordering. */
  if(m.id<27u){
   uint8_t duplicate=0u;
   for(uint8_t j=0u;j<out;j++)if(s->mono_stack[j].id==m.id){duplicate=1u;break;}
   if(duplicate){changed=1u;continue;}
  }
  s->mono_stack[out++]=m;
 }
 if(changed)s->mono_count=out;
 return changed;
}

/* MIDI input overflow/disconnect invalidates the MIDI note stream only.
 * Previously clearing the WHOLE mono stack also discarded physical-key
 * ownership, while leaving voice[0] held if it was a panel note. Its later
 * key-up would then find no matching stack entry, causing a stuck note.
 * Keep physical and sequencer notes; the caller must resume the survivor. */
static inline void ru_mono_midi_panic(struct etch_native_audio *s){
 if(!s)return;
 if(s->mono_count>ETCH_MONO_STACK){s->mono_count=0u;return;}
 uint8_t out=0u;
 for(uint8_t i=0u;i<s->mono_count;i++){
  const struct etch_mono_held m=s->mono_stack[i];
  if(m.id>=128u&&m.id<2176u)continue;
  s->mono_stack[out++]=m;
 }
 s->mono_count=out;
}
#endif
