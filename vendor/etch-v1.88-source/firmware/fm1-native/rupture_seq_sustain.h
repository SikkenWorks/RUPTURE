/* RUPTURE V1.23: bounded monophonic per-step sustain, stored in native note.length.
 * Runs on the control thread only. Does not create triggers or alter velocity. */
#pragma once
#include <stdint.h>
#include "native_performance.h"
static inline uint8_t ru_seq_extend_length(struct etch_native_pattern *p,uint16_t step,
 uint16_t total,int32_t delta){
 if(!p||step>=total||total>ETCH_SEQ_STEPS||!delta)return 0u;
 struct etch_seq_step *origin=&p->steps[step];
 if(!origin->count||origin->count>ETCH_SEQ_NOTES)return 0u;
 uint16_t end=total;
 for(uint16_t j=(uint16_t)(step+1u);j<total;j++)
  if(p->steps[j].count){end=j;break;}
 uint16_t max=(uint16_t)(end-step);if(max>255u)max=255u;
 if(!max)return 0u;
 uint8_t changed=0u;
 for(uint8_t k=0u;k<origin->count;k++){
  int32_t old=origin->notes[k].length?origin->notes[k].length:1;
  int32_t length=old+delta;
  if(length<1)length=1;
  if(length>max)length=max;
  if((uint8_t)length!=origin->notes[k].length){origin->notes[k].length=(uint8_t)length;changed=1u;}
 }
 return changed;
}
static inline uint16_t ru_seq_sustain_mask(const struct etch_native_pattern *p,
 uint16_t total,uint16_t first){
 if(!p||!total||total>ETCH_SEQ_STEPS)return 0u;
 uint16_t mask=0u;
 for(uint16_t step=0u;step<total;step++){
  const struct etch_seq_step *st=&p->steps[step];
  for(uint8_t k=0u;k<st->count&&k<ETCH_SEQ_NOTES;k++){
   uint16_t end=(uint16_t)(step+st->notes[k].length);
   if(end>total)end=total;
   for(uint16_t j=(uint16_t)(step+1u);j<end;j++){
    if(p->steps[j].count)break; /* never draw sustain over an attack */
    if(j>=first&&j<first+16u)mask|=(uint16_t)(1u<<(j-first));
   }
  }
 }
 return mask;
}
