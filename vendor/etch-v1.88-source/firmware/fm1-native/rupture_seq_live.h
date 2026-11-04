/* SPDX-License-Identifier: GPL-3.0-only
 * RUPTURE V1.74 - UI-thread live note sustain + circular pattern move.
 * No DSP/ISR allocation, IO or flash writes. Active note edits are IRQ guarded
 * by the caller; complete pattern rotation builds in the inactive bank.
 */
#ifndef RUPTURE_SEQ_LIVE_H
#define RUPTURE_SEQ_LIVE_H
#include <stdint.h>
#include "native_performance.h"
struct RuLiveCapture {
 uint32_t active,pending; /* pending = quantized to NEXT step, not sounded yet */
 uint16_t origins[27];
 uint16_t elapsed[27];
 uint8_t pitches[27];
 uint16_t prev_step;
 uint8_t valid;
};
static inline void ru_live_reset(struct RuLiveCapture *c){if(c){c->active=0u;c->pending=0u;c->valid=0u;}}
static inline void ru_live_note_on(struct RuLiveCapture *c,uint8_t key,uint16_t step,uint8_t pitch,uint16_t length){
 if(!c||key>=27u||step>=length||pitch>127u)return;
 c->origins[key]=step;c->pitches[key]=pitch;c->elapsed[key]=0u;
 c->active|=1u<<key;c->pending&=~(1u<<key);
 if(!c->valid){c->prev_step=step;c->valid=1u;}
}
static inline uint8_t ru_live_commit(struct etch_native_pattern *p,struct RuLiveCapture *c,uint8_t key){
 if(!c||!p||key>=27u||!(c->active&(1u<<key)))return 0u;
 uint16_t origin=c->origins[key];
 if(origin>=ETCH_SEQ_STEPS)return 0u;
 struct etch_seq_step *s=&p->steps[origin];
 for(uint8_t i=0;i<s->count&&i<ETCH_SEQ_NOTES;i++)if(s->notes[i].note==c->pitches[key]){
  uint16_t duration=c->elapsed[key]+1u;
  if(duration>255u)duration=255u;
  if(duration>s->notes[i].length){s->notes[i].length=(uint8_t)duration;return 1u;}
  return 0u;
 }
 return 0u;
}
/* Keep old note-on API for compatibility with existing RUPTURE host tests.
 * A quantized note at NEXT step must not increment sustain until that step
 * actually sounds. A timestamp-rewound note at PREVIOUS step is already one
 * step old and should preserve its real gate duration. */
static inline void ru_live_note_on_quantized(struct RuLiveCapture *c,uint8_t key,
 uint16_t quant_step,uint8_t pitch,uint16_t length,uint16_t audible_step){
 ru_live_note_on(c,key,quant_step,pitch,length);
 if(!c||key>=27u||!length||audible_step>=length)return;
 if(quant_step==(uint16_t)((audible_step+1u)%length))c->pending|=1u<<key;
 else if(audible_step==(uint16_t)((quant_step+1u)%length))c->elapsed[key]=1u;
}
/* Called on UI tick with current sequencer playhead. Calculate actual traversed
 * step count even if UI misses multiple boundaries. Finishing a note on key
 * release never overwrites a longer existing overdub or another note. */
static inline uint8_t ru_live_advance(struct etch_native_pattern *p,struct RuLiveCapture *c,
 uint16_t step,uint16_t length,uint32_t keys_down){
 if(!c||!p||!length||length>ETCH_SEQ_STEPS||step>=length)return 0u;
 if(!c->valid){c->prev_step=step;c->valid=1u;}
 uint16_t prior=c->prev_step;
 uint16_t delta=(uint16_t)((step+length-prior)%length);
 c->prev_step=step;
 uint8_t changed=0u;
 for(uint8_t key=0u;key<27u;key++)if(c->active&(1u<<key)){
  uint32_t bit=1u<<key;
  if(c->pending&bit){
   uint16_t until=(uint16_t)((c->origins[key]+length-prior)%length);
   if(!delta||until>delta){
    if(!(keys_down&bit))c->active&=~bit;
    continue;
   }
   c->pending&=~bit;
   c->elapsed[key]=(uint16_t)(delta-until);
  }else{
   uint32_t duration=(uint32_t)c->elapsed[key]+delta;
   c->elapsed[key]=(uint16_t)(duration>254u?254u:duration);
  }
  /* Publish length while held, not just on Note OFF, so HOLD is visible.
   * This covers recording until the user stops transport without releasing. */
  if(ru_live_commit(p,c,key))changed=1u;
  if(!(keys_down&(1u<<key)))c->active&=~(1u<<key);
 }
 return changed;
}
/* For shutdown / page navigation, terminate all held capture instances. */
static inline uint8_t ru_live_finish(struct etch_native_pattern *p,struct RuLiveCapture *c){
 uint8_t changed=0u;
 if(!c||!p)return 0u;
 for(uint8_t key=0u;key<27u;key++)if(c->active&(1u<<key))changed|=ru_live_commit(p,c,key);
 ru_live_reset(c);return changed;
}
/* Pure, transactional transformation on a STAGING buffer. +1 means later
 * (right); -1 earlier (left). Preserve every step, including parameter locks
 * and linked HOLD origin identities across the wrapping boundary. */
static inline uint8_t ru_seq_rotate(const struct etch_native_pattern *src,
 struct etch_native_pattern *dst,uint16_t length,int32_t detents){
 if(!src||!dst||src==dst||length<2u||length>ETCH_SEQ_STEPS||!detents)return 0u;
 int32_t movement=detents%(int32_t)length;
 if(movement<0)movement+=(int32_t)length;
 if(!movement)return 0u;
 /* Copy the active pattern while the audio reads it, then rearrange ONLY
  * the inactive bank. No interrupt masking is needed for this stage. */
 *dst=*src;
 for(uint16_t old=0u;old<length;old++){
  uint16_t shifted=(uint16_t)((old+(uint16_t)movement)%length);
  dst->steps[shifted]=src->steps[old];
  struct etch_seq_step *s=&dst->steps[shifted];
  for(uint8_t k=0u;k<s->hold_count&&k<ETCH_SEQ_NOTES;k++)if(s->hold_ago[k]){
   uint16_t prior=(uint16_t)((old+256u-s->hold_ago[k])&255u);
   if(prior<length){
    uint16_t new_origin=(uint16_t)((prior+(uint16_t)movement)%length);
    s->hold_ago[k]=(uint8_t)((shifted+256u-new_origin)&255u);
   }
  }
 }
 return 1u;
}
#endif
