/* SPDX-License-Identifier: GPL-3.0-only
 * RUPTURE V2.0.18: lower-budget CXS SUB audio, FM1 mono audio engine only.
 *
 * The SUB is an octave bass layer. Calculate all active SUB oscillators at
 * 11.025kHz and linearly interpolate consecutive values in
 * the 44.1kHz DMA stream. Its pitch remains correct (twice the accumulator
 * increment); original main CXS and GROWL DSP remain full-rate. OFF remains
 * a very cheap per-sample control path. This is an intentional band-limited
 * SUB-quality tradeoff, not PCM-identical to the old active SUB oscillators,
 * but protects real-time bandwidth without dropping DMA frames.
 * The interpolation state belongs to the one mono audio engine in ordinary
 * BSS (never the saturated DSP POOL); caller resets it on note-on/panic.
 */
#ifndef RUPTURE_SUB_REALTIME_H
#define RUPTURE_SUB_REALTIME_H
#include "rupture_subosc.h"

typedef struct {
 int16_t from,to;
 uint8_t phase;
} RuSubRt;

static inline void ru_sub_rt_reset(RuSubRt *r){r->from=r->to=0;r->phase=0u;}

static inline int32_t ru_sub_rt_tick(RuSubRt *r,RuSubState *s,
 uint8_t type,uint8_t oct,uint8_t detune,uint8_t drift,uint32_t base,
 uint8_t pressure,uint8_t balance,uint8_t memory,uint8_t density){
 /* No costly oscillator while OFF; bypass at extreme pitch increments
  * that cannot safely be quadrupled in uint32_t. */
 if((type==RU_SUB_OFF&&s->running_type==RU_SUB_OFF)||base>0x3fffffffu){
  ru_sub_rt_reset(r);
  return ru_sub_tick(s,type,oct,detune,drift,base,
                     pressure,balance,memory,density);
 }
 if(!s->running_type){ru_sub_rt_reset(r);}
 if(!r->phase){
  r->from=r->to;
  int32_t old_smooth=s->cxs.smooth;
  r->to=(int16_t)ru_sub_tick(s,type,oct,detune,drift,base*4u,
                            pressure,balance,memory,density);
  if(s->running_type==RU_SUB_CXS){
   /* Equalize the original 4x 1/4-pole response in one low-rate update:
    * 1-(3/4)^4 = 175/256, vs one ordinary update of 1/4.
    * Relative to the just-computed change: 175/64, with bounded Q15 state.
    * This keeps the low SUB body and upper-grit energy near the full-rate
    * original without computing three extra CXS oscillator samples. */
   int32_t adjusted=old_smooth+((s->cxs.smooth-old_smooth)*175)/64;
   if(adjusted>32767)adjusted=32767;
   if(adjusted< -32767)adjusted=-32767;
   s->cxs.smooth=adjusted;
   r->to=(int16_t)adjusted;
  }
  r->phase=1u;
  return (int32_t)r->from+(((int32_t)r->to-(int32_t)r->from)/4);
 }
 int32_t sample=(int32_t)r->from+
   ((((int32_t)r->to-(int32_t)r->from)*(int32_t)(r->phase+1u))/4);
 r->phase=(uint8_t)((r->phase+1u)&3u);
 return sample;
}
#endif
