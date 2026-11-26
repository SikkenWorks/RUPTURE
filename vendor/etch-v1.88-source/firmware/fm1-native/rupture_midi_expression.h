/* SPDX-License-Identifier: GPL-3.0-only
 * RUPTURE V2.0.58 MIDI expression. Observation only: MIDI note ownership,
 * retriggers, sustain, overflow and Note OFF remain in the native engine.
 * All helpers run at an ALNK BLOCK boundary, never per audio sample.
 */
#ifndef RUPTURE_MIDI_EXPRESSION_H
#define RUPTURE_MIDI_EXPRESSION_H
#include <stdint.h>
#include "audio_core.h"
#include "rupture_touch_profile.h"

enum {RU_MIDI_EXPR_NONE,RU_MIDI_EXPR_VELOCITY,
      RU_MIDI_EXPR_CHANNEL_PRESSURE,RU_MIDI_EXPR_POLY_PRESSURE};
enum {RU_MIDI_EXPR_SLOTS=8u};
struct ru_midi_expression {
 uint8_t channel_pressure[16];
 uint16_t channel_seen;
 uint16_t poly_id[RU_MIDI_EXPR_SLOTS];
 uint8_t poly_pressure[RU_MIDI_EXPR_SLOTS];
 uint8_t poly_valid[RU_MIDI_EXPR_SLOTS];
 uint8_t replacement;
};
static inline void ru_midi_expr_reset(struct ru_midi_expression *s){
 if(!s)return;
 s->channel_seen=0u;s->replacement=0u;
 for(uint8_t c=0u;c<16u;c++)s->channel_pressure[c]=0u;
 for(uint8_t i=0u;i<RU_MIDI_EXPR_SLOTS;i++){
  s->poly_id[i]=0u;s->poly_pressure[i]=0u;s->poly_valid[i]=0u;
 }
}
static inline void ru_midi_expr_drop_id(struct ru_midi_expression *s,uint16_t id){
 for(uint8_t i=0u;i<RU_MIDI_EXPR_SLOTS;i++)
  if(s->poly_valid[i]&&s->poly_id[i]==id)s->poly_valid[i]=0u;
}
static inline void ru_midi_expr_drop_channel(struct ru_midi_expression *s,uint8_t ch){
 s->channel_seen&=(uint16_t)~(1u<<ch);
 s->channel_pressure[ch]=0u;
 for(uint8_t i=0u;i<RU_MIDI_EXPR_SLOTS;i++)
  if(s->poly_valid[i]&&((s->poly_id[i]-128u)>>7)==ch)s->poly_valid[i]=0u;
}
/* MIDI packet is observed after native_midi() sees it. All accepted USB and
 * TRS MIDI inputs share this queue; no note event is swallowed or reordered. */
static inline void ru_midi_expr_event(struct ru_midi_expression *s,
 uint8_t status,uint8_t d1,uint8_t d2){
 if(!s||status<0x80u||status>=0xf0u||d1>127u||d2>127u)return;
 uint8_t kind=status&0xf0u,ch=status&15u;
 uint16_t id=(uint16_t)(128u+(uint16_t)ch*128u+d1);
 if(kind==0xd0u){
  s->channel_pressure[ch]=d1;s->channel_seen|=(uint16_t)(1u<<ch);
 }else if(kind==0xa0u){
  for(uint8_t i=0u;i<RU_MIDI_EXPR_SLOTS;i++)if(s->poly_valid[i]&&s->poly_id[i]==id){
   s->poly_pressure[i]=d2;return;
  }
  uint8_t slot=RU_MIDI_EXPR_SLOTS;
  for(uint8_t i=0u;i<RU_MIDI_EXPR_SLOTS;i++)if(!s->poly_valid[i]){slot=i;break;}
  if(slot==RU_MIDI_EXPR_SLOTS){slot=s->replacement;
   s->replacement=(uint8_t)((s->replacement+1u)%RU_MIDI_EXPR_SLOTS);}
  s->poly_id[slot]=id;s->poly_pressure[slot]=d2;s->poly_valid[slot]=1u;
 }else if(kind==0x80u||(kind==0x90u&&!d2)||kind==0x90u){
  /* A new strike discards the previous aftertouch for that same pitch. */
  ru_midi_expr_drop_id(s,id);
 }else if(kind==0xb0u){
  if(d1==120u||d1==121u||d1==123u)ru_midi_expr_drop_channel(s,ch);
 }
}
static inline uint8_t ru_midi_expr_held(const struct etch_native_audio *a){
 return (uint8_t)(a&&a->voice[0].active&&a->voice[0].held&&
  a->voice[0].note>=128u&&a->voice[0].note<2176u);
}
/* Values 1..127 map to exactly 0..127 before the selected curve.
 * SOFT/LINEAR/HARD/EXTRA-HARD are fixed integer maps, zero added DSP math. */
static inline uint8_t ru_midi_velocity_level(uint8_t vel,uint8_t feel){
 if(!vel)return 0u;
 uint32_t x=((uint32_t)(vel-1u)*127u+63u)/126u;
 if(feel==0u)return (uint8_t)((x*(254u-x)+63u)/127u);
 if(feel==2u)return (uint8_t)((x*x+63u)/127u);
 if(feel==3u)return (uint8_t)((x*x*x+8064u)/16129u);
 return (uint8_t)x;
}
/* Returns normalized expression and a physical data-source label. Poly AT
 * wins over channel AT; if the keyboard has no AT, PRESS falls back to its
 * ordinary Note On velocity. VELOCITY ALWAYS uses the Note On velocity. */
static inline uint8_t ru_midi_expr_level(const struct ru_midi_expression *s,
 const struct etch_native_audio *a,const struct ru_touch_profile *p,
 uint8_t *source,uint8_t *raw){
 if(source)*source=RU_MIDI_EXPR_NONE;
 if(raw)*raw=0u;
 if(!s||!p||!ru_midi_expr_held(a))return 0u;
 const struct etch_native_voice *v=&a->voice[0];
 uint8_t ch=(uint8_t)((v->note-128u)>>7);
 if(p->mode==RU_TOUCH_MODE_PRESS){
  for(uint8_t i=0u;i<RU_MIDI_EXPR_SLOTS;i++)
   if(s->poly_valid[i]&&s->poly_id[i]==v->note){
    if(source)*source=RU_MIDI_EXPR_POLY_PRESSURE;
    if(raw)*raw=s->poly_pressure[i];
    return s->poly_pressure[i];
   }
  if(s->channel_seen&(1u<<ch)){
   if(source)*source=RU_MIDI_EXPR_CHANNEL_PRESSURE;
   if(raw)*raw=s->channel_pressure[ch];
   return s->channel_pressure[ch];
  }
 }
 if(source)*source=RU_MIDI_EXPR_VELOCITY;
 if(raw)*raw=v->velocity;
 return ru_midi_velocity_level(v->velocity,p->sensitivity);
}
#endif
