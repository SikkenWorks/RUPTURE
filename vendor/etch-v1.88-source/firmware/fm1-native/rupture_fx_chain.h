/* RUPTURE V1.94: eight editable FX slots with independently stored controls.
 * No large delay/chorus/reverb PCM buffers are added to the audio engine. */
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
/* Browser order only. Stored type IDs and the audible chain stay intact. */
static const uint8_t ru_fx_library_order[RU_FX_TYPES]={
 RU_FX_REVERB,RU_FX_DELAY,RU_FX_CHORUS,RU_FX_WAH,
 RU_FX_MOSFET,RU_FX_DISTORTION,RU_FX_FUZZ,RU_FX_SAND,
 RU_FX_COMP,RU_FX_TILT
};
static inline uint8_t ru_fx_library_index(uint8_t type){
 for(uint8_t i=0u;i<RU_FX_TYPES;i++)if(ru_fx_library_order[i]==type)return i;
 return 0u;
}
static inline uint8_t ru_fx_library_type(uint8_t pos){
 return pos<RU_FX_TYPES?ru_fx_library_order[pos]:RU_FX_EMPTY;
}
struct rupture_fx_chain {
 uint8_t type[RU_FX_SLOTS],enabled;
 /* Four native-unit knobs per insert, separate from all other inserts.
  * independent==0 preserves the V1.93 global-parameter interpretation for
  * projects saved before this extension. */
 uint8_t independent;
 uint16_t parameter[RU_FX_SLOTS][4];
};
/* A chain snapshot is fixed for an entire 128-frame ALNK0 render. Trailing
 * EMPTY slots have no fade, DSP state or tank work; keep their scan out of
 * the sample loop. Interior EMPTY slots must stay in their original order. */
static inline uint8_t ru_chain_span(const struct rupture_fx_chain *c){
 if(!c)return 0u;
 for(uint8_t n=RU_FX_SLOTS;n>0u;n--)
  if(c->type[n-1u]!=RU_FX_EMPTY)return n;
 return 0u;
}
/* V2.0.22: an immutable, eight-byte slot list for one published ALNK block.
 * Include bypassed effects: their fade and feedback tails still need work.
 * EMPTY positions have neither state nor audio work and can be skipped even
 * when a later occupied slot keeps the chain span at eight. */
struct rupture_fx_dispatch {
 uint8_t span,count;
 uint8_t slot[RU_FX_SLOTS];
};
static inline struct rupture_fx_dispatch ru_chain_prepare_dispatch(const struct rupture_fx_chain *c){
 struct rupture_fx_dispatch d={0};
 if(!c)return d;
 for(uint8_t n=0u;n<RU_FX_SLOTS;n++)if(c->type[n]!=RU_FX_EMPTY){
  d.slot[d.count++]=n;
  d.span=(uint8_t)(n+1u);
 }
 return d;
}
static inline void ru_chain_slot_defaults(struct rupture_fx_chain *c,uint8_t slot,uint8_t type){
 if(slot>=RU_FX_SLOTS)return;
 uint16_t *p=c->parameter[slot];
 p[0]=p[1]=p[2]=p[3]=0u;
 switch(type){
 case RU_FX_REVERB:p[0]=1u;p[1]=20480u;p[2]=32767u;p[3]=12000u;break;
 case RU_FX_DELAY:p[0]=16384u;p[1]=12288u;p[3]=12000u;break;
 case RU_FX_CHORUS:p[0]=4096u;p[1]=16384u;p[3]=12000u;break;
 case RU_FX_MOSFET:p[0]=8000u;break;
 case RU_FX_COMP:p[0]=16384u;p[1]=11000u;p[2]=8192u;p[3]=16384u;break;
 case RU_FX_TILT:p[0]=50u;p[1]=50u;p[2]=65u;p[3]=50u;break;
 case RU_FX_FUZZ:p[0]=55u;p[1]=75u;p[2]=65u;p[3]=65u;break;
 case RU_FX_SAND:p[0]=60u;p[1]=40u;p[2]=45u;p[3]=80u;break;
 case RU_FX_DISTORTION:p[0]=23500u;p[1]=16384u;p[2]=22000u;p[3]=22000u;break;
 case RU_FX_WAH:p[0]=24000u;p[1]=16384u;p[2]=8192u;p[3]=23500u;break;
 default:break;
 }
}
static inline void ru_chain_clear(struct rupture_fx_chain *c){
 for(uint8_t n=0;n<RU_FX_SLOTS;n++){
  c->type[n]=RU_FX_EMPTY;
  for(uint8_t k=0;k<4u;k++)c->parameter[n][k]=0u;
 }
 c->enabled=0u;c->independent=1u;
}
static inline uint8_t ru_chain_valid(const struct rupture_fx_chain *c){
 for(uint8_t n=0;n<RU_FX_SLOTS;n++){
  uint8_t t=c->type[n];if(t==RU_FX_EMPTY)continue;
  if(t>=RU_FX_TYPES)return 0u;
 }
 for(uint8_t n=0;n<RU_FX_SLOTS;n++)if(c->type[n]==RU_FX_EMPTY&&(c->enabled&(1u<<n)))return 0u;
 if(c->independent>1u)return 0u;
 if(c->independent)for(uint8_t n=0;n<RU_FX_SLOTS;n++){
  uint8_t t=c->type[n];if(t==RU_FX_EMPTY)continue;
  for(uint8_t k=0;k<4u;k++){
   uint16_t v=c->parameter[n][k];
   if(v>32767u)return 0u;
   if((t==RU_FX_TILT||t==RU_FX_FUZZ||t==RU_FX_SAND)&&v>100u)return 0u;
  }
  if(t==RU_FX_REVERB&&c->parameter[n][0]>2u)return 0u;
  if((t==RU_FX_DELAY||t==RU_FX_CHORUS)&&c->parameter[n][2]>1u)return 0u;
 }
 return 1u;
}
static inline uint8_t ru_chain_insert(struct rupture_fx_chain *c,uint8_t slot,uint8_t type){
 if(slot>=RU_FX_SLOTS||type>=RU_FX_TYPES)return 0u;
 if(c->type[slot]!=type)ru_chain_slot_defaults(c,slot,type);
 c->type[slot]=type;c->enabled|=(uint8_t)(1u<<slot);return 1u;
}
static inline void ru_chain_remove(struct rupture_fx_chain *c,uint8_t slot){
 if(slot>=RU_FX_SLOTS)return;
 c->type[slot]=RU_FX_EMPTY;
 c->enabled&=(uint8_t)~(1u<<slot);
 for(uint8_t k=0;k<4u;k++)c->parameter[slot][k]=0u;
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
 for(uint8_t k=0;k<4u;k++){
  uint16_t value=c->parameter[slot][k];
  c->parameter[slot][k]=c->parameter[other][k];c->parameter[other][k]=value;
 }
 uint8_t a=(uint8_t)((c->enabled>>slot)&1u),b=(uint8_t)((c->enabled>>other)&1u);
 c->enabled=(uint8_t)((c->enabled&~((1u<<slot)|(1u<<other)))|(a<<other)|(b<<slot));
 return 1u;
}
#endif
