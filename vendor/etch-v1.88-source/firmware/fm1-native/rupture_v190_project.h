/* PJT35 project extension. No changes to old project record layouts. */
#ifndef RUPTURE_V190_PROJECT_H
#define RUPTURE_V190_PROJECT_H
#include <stdint.h>
#include "rupture_fx_chain.h"
struct rupture_v190_project {
 struct rupture_fx_chain chain; /* nine bytes: eight processors + enable */
 uint8_t legacy_chain; /* 1: byte-identical old DSP order until first slot edit */
 uint8_t glide_patch[32][2]; /* per preset: 0..200 * 10ms, mode/3-bit curve/retrigger */
 uint8_t seq_slide[32]; /* 256 steps, one SLIDE flag per step */
};
static inline void ru_v190_defaults(struct rupture_v190_project *p){
 ru_chain_clear(&p->chain);
 p->legacy_chain=0u;
 for(uint8_t i=0u;i<32u;i++){
  p->glide_patch[i][0]=0u;p->glide_patch[i][1]=0x10u; /* retrigger ON */
  p->seq_slide[i]=0u;
 }
}
/* Stage one preset's complete GLIDE settings in the existing PJT35 bytes.
 * This is RAM-only; the project journal commits later on manual/auto SAVE. */
static inline uint8_t ru_v190_glide_stage(struct rupture_v190_project *p,
 uint8_t slot,uint16_t time_ms,uint8_t mode,uint8_t curve,uint8_t retrigger){
 if(!p||slot>=32u)return 0u;
 if(time_ms>2000u)time_ms=2000u;
 if(mode>2u)mode=2u;
 uint8_t time=(uint8_t)(time_ms/10u);
 if(curve>5u)curve=5u;
 /* Original curve bit stays at bit 2; use reserved bits 3/5 for 2..5. */
 uint8_t flags=(uint8_t)((mode&3u)|((curve&3u)<<2u)|((curve&4u)<<3u)|((retrigger&1u)<<4u));
 uint8_t changed=(uint8_t)(p->glide_patch[slot][0]!=time||
                           p->glide_patch[slot][1]!=flags);
 p->glide_patch[slot][0]=time;
 p->glide_patch[slot][1]=flags;
 return changed;
}
static inline uint8_t ru_v190_valid(const struct rupture_v190_project *p){
 if(!ru_chain_valid(&p->chain))return 0u;
 if(p->legacy_chain>1u)return 0u;
 for(uint8_t i=0u;i<32u;i++)
  if(p->glide_patch[i][0]>200u||(p->glide_patch[i][1]&~0x3fu)||
      (p->glide_patch[i][1]&3u)>2u||
      (((p->glide_patch[i][1]>>2u)&3u)|((p->glide_patch[i][1]>>3u)&4u))>5u)return 0u;
 return 1u;
}
static inline uint8_t ru_slide_at(const struct rupture_v190_project *p,uint16_t step){
 return step<256u?(uint8_t)!!(p->seq_slide[step>>3u]&(1u<<(step&7u))):0u;
}
static inline void ru_slide_set(struct rupture_v190_project *p,uint16_t step,uint8_t yes){
 if(step>=256u)return;
 uint8_t bit=(uint8_t)(1u<<(step&7u));
 if(yes)p->seq_slide[step>>3u]|=bit;
 else p->seq_slide[step>>3u]&=(uint8_t)~bit;
}
#endif
