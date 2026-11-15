/* RUPTURE V1.93: eight editable post-mix FX slots.
 * Effect types may repeat. Large time-based processors retain one shared
 * tank each; their wet return is rendered once per PCM frame and can be
 * inserted again without advancing the same ring multiple times. */
#ifndef RUPTURE_FX_CHAIN_H
#define RUPTURE_FX_CHAIN_H
#include <stdint.h>
#define RU_FX_SLOTS 8u
#define RU_FX_TYPES 10u /* Eight simultaneous slots; ten choices in the library. */
#define RU_FX_REMOVE RU_FX_TYPES
#define RU_FX_EMPTY 0xffu
enum {RU_FX_REVERB, RU_FX_DELAY, RU_FX_CHORUS, RU_FX_MOSFET,
      RU_FX_COMP, RU_FX_TILT, RU_FX_FUZZ, RU_FX_SAND,
      RU_FX_DISTORTION, RU_FX_WAH};
struct rupture_fx_chain {uint8_t type[RU_FX_SLOTS],enabled;};
static inline void ru_chain_clear(struct rupture_fx_chain *c){
 for(uint8_t n=0;n<RU_FX_SLOTS;n++)c->type[n]=RU_FX_EMPTY;
 c->enabled=0u;
}
static inline uint8_t ru_chain_valid(const struct rupture_fx_chain *c){
 for(uint8_t n=0;n<RU_FX_SLOTS;n++){
  uint8_t t=c->type[n];if(t==RU_FX_EMPTY)continue;
  if(t>=RU_FX_TYPES)return 0u;
 }
 for(uint8_t n=0;n<RU_FX_SLOTS;n++)if(c->type[n]==RU_FX_EMPTY&&(c->enabled&(1u<<n)))return 0u;
 return 1u;
}
static inline uint8_t ru_chain_insert(struct rupture_fx_chain *c,uint8_t slot,uint8_t type){
 if(slot>=RU_FX_SLOTS||type>=RU_FX_TYPES)return 0u;
 c->type[slot]=type;c->enabled|=(uint8_t)(1u<<slot);return 1u;
}
static inline void ru_chain_remove(struct rupture_fx_chain *c,uint8_t slot){
 if(slot>=RU_FX_SLOTS)return;
 c->type[slot]=RU_FX_EMPTY;
 c->enabled&=(uint8_t)~(1u<<slot);
}
static inline void ru_chain_toggle(struct rupture_fx_chain *c,uint8_t slot){
 if(slot<RU_FX_SLOTS&&c->type[slot]!=RU_FX_EMPTY)c->enabled^=(uint8_t)(1u<<slot);
}
/* Adjacent swap moves both TYPE and ENABLE state; no algorithm re-sort. */
static inline uint8_t ru_chain_move(struct rupture_fx_chain *c,uint8_t slot,int8_t direction){
 int dest=(int)slot+direction;
 if(slot>=RU_FX_SLOTS||dest<0||dest>=(int)RU_FX_SLOTS)return 0u;
 uint8_t other=(uint8_t)dest,tmp=c->type[slot];
 c->type[slot]=c->type[other];c->type[other]=tmp;
 uint8_t a=(uint8_t)((c->enabled>>slot)&1u),b=(uint8_t)((c->enabled>>other)&1u);
 c->enabled=(uint8_t)((c->enabled&~((1u<<slot)|(1u<<other)))|(a<<other)|(b<<slot));
 return 1u;
}
#endif
