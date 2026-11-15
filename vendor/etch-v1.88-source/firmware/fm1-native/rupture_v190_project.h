/* PJT35 project extension. No changes to old project record layouts. */
#ifndef RUPTURE_V190_PROJECT_H
#define RUPTURE_V190_PROJECT_H
#include <stdint.h>
#include "rupture_fx_chain.h"
struct rupture_v190_project {
 struct rupture_fx_chain chain; /* nine bytes: eight processors + enable */
 uint8_t legacy_chain; /* 1: byte-identical old DSP order until first slot edit */
 uint8_t glide_patch[32][2]; /* per preset: 0..200 * 10ms, mode/curve/retrigger */
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
static inline uint8_t ru_v190_valid(const struct rupture_v190_project *p){
 if(!ru_chain_valid(&p->chain))return 0u;
 if(p->legacy_chain>1u)return 0u;
 for(uint8_t i=0u;i<32u;i++)
  if(p->glide_patch[i][0]>200u||(p->glide_patch[i][1]&~0x17u)||
      (p->glide_patch[i][1]&3u)>2u)return 0u;
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
