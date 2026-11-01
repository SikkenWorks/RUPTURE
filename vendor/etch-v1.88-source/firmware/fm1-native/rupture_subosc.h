/* SPDX-License-Identifier: GPL-3.0-only
 * RUPTURE 1.9: dual independent mono suboscillators, all fixed point.
 * Type OFF/CXS/CLASSIC; classic = harmonically band-limited square-like sub, CXS = separate 8-way exchange state.
 * No heap/flash I/O in audio callback; stable fade/gain boundaries.
 */
#ifndef RUPTURE_SUBOSC_H
#define RUPTURE_SUBOSC_H
#include <stdint.h>
#include "native_cxs_q15.h"
#include "rupture_bandwaves.h"
#define RU_SUB_OFF 0u
#define RU_SUB_CXS 1u
#define RU_SUB_CLASSIC 2u
#define RU_SUB_SQUARE RU_WAVE_SQUARE
#define RU_SUB_SAW RU_WAVE_SAW
#define RU_SUB_MIX_MAX 16384u /* each sub up to 50%; main always unity before peak guard */
typedef struct {
 RuptureCXSQ15 cxs;
 uint32_t phase,seed;
 int32_t drift_q16,drift_target_q16,drift_residue;
 uint16_t mix,blend_smooth;
 uint8_t running_type;
} RuSubState;
static const int16_t ru_sub_sine[257]={0,804,1608,2410,3212,4011,4808,5602,6393,7179,7962,8739,9512,10278,11039,11793,12539,13279,14010,14732,15446,16151,16846,17530,18204,18868,19519,20159,20787,21403,22005,22594,23170,23731,24279,24811,25329,25832,26319,26790,27245,27683,28105,28510,28898,29268,29621,29956,30273,30571,30852,31113,31356,31580,31785,31971,32137,32285,32412,32521,32609,32678,32728,32757,32767,32757,32728,32678,32609,32521,32412,32285,32137,31971,31785,31580,31356,31113,30852,30571,30273,29956,29621,29268,28898,28510,28105,27683,27245,26790,26319,25832,25329,24811,24279,23731,23170,22594,22005,21403,20787,20159,19519,18868,18204,17530,16846,16151,15446,14732,14010,13279,12539,11793,11039,10278,9512,8739,7962,7179,6393,5602,4808,4011,3212,2410,1608,804,0,-804,-1608,-2410,-3212,-4011,-4808,-5602,-6393,-7179,-7962,-8739,-9512,-10278,-11039,-11793,-12539,-13279,-14010,-14732,-15446,-16151,-16846,-17530,-18204,-18868,-19519,-20159,-20787,-21403,-22005,-22594,-23170,-23731,-24279,-24811,-25329,-25832,-26319,-26790,-27245,-27683,-28105,-28510,-28898,-29268,-29621,-29956,-30273,-30571,-30852,-31113,-31356,-31580,-31785,-31971,-32137,-32285,-32412,-32521,-32609,-32678,-32728,-32757,-32767,-32757,-32728,-32678,-32609,-32521,-32412,-32285,-32137,-31971,-31785,-31580,-31356,-31113,-30852,-30571,-30273,-29956,-29621,-29268,-28898,-28510,-28105,-27683,-27245,-26790,-26319,-25832,-25329,-24811,-24279,-23731,-23170,-22594,-22005,-21403,-20787,-20159,-19519,-18868,-18204,-17530,-16846,-16151,-15446,-14732,-14010,-13279,-12539,-11793,-11039,-10278,-9512,-8739,-7962,-7179,-6393,-5602,-4808,-4011,-3212,-2410,-1608,-804,0};
static inline uint32_t ru_sub_rng(uint32_t *seed){uint32_t x=*seed;x^=x<<13;x^=x>>17;x^=x<<5;*seed=x;return x;}
static inline void ru_sub_init(RuSubState *s,uint32_t inc,uint32_t seed){
 uint8_t *p=(uint8_t*)s;for(uint32_t i=0;i<sizeof(*s);i++)p[i]=0;
 s->seed=seed?seed:0xa9c63f29u;rupture_q15_init(&s->cxs,inc);
 s->cxs.seed=seed^0x61230af5u;
}
/* octave index 0=-2,1=-1,2=0,3=+1. 50 detune = neutral.
 * 38 Q16 units ~= 1 cent over +/-50 cent range, 32-bit math throughout
 * apart from the same bounded 32x32 ->64 multiply already used by CXS. */
static inline uint32_t ru_sub_inc(uint32_t base,uint8_t octave,uint8_t detune,int32_t drift){
 uint64_t inc=base;
 if(octave==0u)inc>>=2u;else if(octave==1u)inc>>=1u;
 else if(octave==3u)inc<<=1u;
 int32_t cents=(int32_t)detune-50;
 int32_t factor=65536+cents*38+drift;
 if(factor<32768)factor=32768;
 inc=(inc*(uint32_t)factor)>>16;
 return (uint32_t)(inc>0xffffffffULL?0xffffffffULL:inc<1u?1u:inc);
}
static inline int16_t ru_sub_sine_read(uint32_t phase){
 uint32_t i=phase>>24,fract=(phase>>8)&65535u;
 int32_t a=ru_sub_sine[i],b=ru_sub_sine[i+1u];
 return (int16_t)(a+((b-a)*(int32_t)fract>>16));
}
/* CLASSIC: a square-like low oscillator built from odd sine harmonics.
 * The old pure sine vanished on small speakers at low octave settings.
 * Harmonics are omitted when their frequency would approach Nyquist.
 * This preserves the fundamental while giving classic bass presence. */
static inline int32_t ru_sub_classic_read(uint32_t phase,uint32_t increment){
 int32_t output=((int32_t)ru_sub_sine_read(phase)*21350)>>15;
 if(increment<=(0x7fffffffu/3u))
   output+=((int32_t)ru_sub_sine_read(phase*3u)*7116)>>15;
 if(increment<=(0x7fffffffu/5u))
   output+=((int32_t)ru_sub_sine_read(phase*5u)*4270)>>15;
 if(output>32767)output=32767;
 if(output< -32768)output=-32768;
 return output;
}
static inline int32_t ru_sub_tick(RuSubState *s,uint8_t type,uint8_t oct,uint8_t detune,uint8_t drift,
 uint32_t base,uint8_t pressure,uint8_t balance,uint8_t memory,uint8_t density){
 if(type>4u)type=0u;
 /* Before a TYPE switch fade down the old voice, avoiding hard swaps. */
 if(s->running_type!=type && s->mix==0u){
  s->running_type=type;s->drift_q16=s->drift_target_q16=s->drift_residue=0;
  s->phase=0u;rupture_q15_init(&s->cxs,base);
  s->cxs.seed=s->seed^0xa9521873u;
 }
 uint16_t goal=(type && s->running_type==type)?RU_SUB_MIX_MAX:0u;
 if(s->mix<goal){s->mix=(uint16_t)(s->mix+256u>goal?goal:s->mix+256u);}
 else if(s->mix>goal){s->mix=(uint16_t)(s->mix>goal+256u?s->mix-256u:goal);}
 if(!s->running_type||!s->mix)return 0;
 uint32_t increment=ru_sub_inc(base,oct,detune,s->drift_q16);
 uint32_t last=s->phase;s->phase+=increment;
 if(s->phase<last){
  /* ± up to 16 cents slow bounded drift; independently seeded per sub. */
  int32_t rnd=(int32_t)(ru_sub_rng(&s->seed)&255u)-128;
  s->drift_target_q16=(rnd*(int32_t)drift*6)/100;
 }
 /* Carry fractional error, otherwise low drift gets stuck at zero. */
 s->drift_residue+=s->drift_target_q16-s->drift_q16;
 int32_t drift_step=s->drift_residue/256;
 s->drift_residue-=drift_step*256;
 s->drift_q16+=drift_step;
 if(s->running_type==RU_SUB_CLASSIC)return ru_sub_classic_read(s->phase,increment);
 if(s->running_type==RU_SUB_SQUARE||s->running_type==RU_SUB_SAW)
  return ru_bandwave_read(s->running_type,s->phase,increment);
 s->cxs.increment=increment;
 rupture_q15_params(&s->cxs,pressure,balance,memory,density);
 s->cxs.edge=50u;
 return rupture_q15_sample(&s->cxs);
}
/* A knob can change by several ADC steps at a time; smooth the actual
 * per-sub blend in the audio callback, NOT on the UI thread. 64 Q15 steps
 * per sample yields about 12ms full-range movement at 44.1kHz. */
static inline uint16_t ru_sub_smooth_blend(RuSubState *s,uint8_t amount){
 if(amount>100u)amount=100u;
 uint32_t target=((uint32_t)s->mix*amount)/100u;
 uint32_t curr=s->blend_smooth;
 if(curr<target){curr+=64u;if(curr>target)curr=target;}
 else if(curr>target){curr=curr>target+64u?curr-64u:target;}
 s->blend_smooth=(uint16_t)curr;
 return s->blend_smooth;
}
/* Additive sub mix: NEVER turn down the CXS carrier when subs are enabled.
 * With both subs OFF this function returns exactly main, preserving V1.9 PCM.
 * The 28k knee operates only on peaks, preventing fixed gain loss.
 * All arithmetic bounded (no 64-bit division in the real-time callback). */
static inline int32_t ru_sub_blend(int32_t main,int32_t a,int32_t b,uint16_t gain_a,uint16_t gain_b){
 if((!gain_a && !gain_b)||(!a && !b))return main;
 if(gain_a>RU_SUB_MIX_MAX)gain_a=RU_SUB_MIX_MAX;
 if(gain_b>RU_SUB_MIX_MAX)gain_b=RU_SUB_MIX_MAX;
 /* 32-bit safe: each product <= 32768*16384, sum after shifts <= 65536. */
 if(a>32767)a=32767;
 if(a< -32768)a=-32768;
 if(b>32767)b=32767;
 if(b< -32768)b=-32768;
 int32_t mix=main+((a*(int32_t)gain_a)>>15)+((b*(int32_t)gain_b)>>15);
 /* At extreme dual-sub blends reserve at most 12.5% before the FX chain.
  * No compensation when subs are absent. No variable division/64-bit math. */
 int32_t atten=((int32_t)(gain_a+gain_b)*4096)>>15;
 mix-=((mix*atten)>>15);
 /* A continuous three-stage knee, no hard full-scale plateau. Under 22k,
  * retain the full low-level body. All intermediates bounded to <100k. */
 int32_t mag=mix<0?-mix:mix;
 if(mag>22000){
  if(mag<=30000)mag=22000+((mag-22000)>>1);
  else mag=26000+((mag-30000)>>4);
  if(mag>28300)mag=28300;
 }
 return mix<0?-mag:mag;
}
#endif
