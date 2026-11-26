/* Host proof: MIDI expression is bounded, source-aware, and never owns notes. */
#define RUPTURE_BUILD 1
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_key_touch.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_midi_expression.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_trs_input.h"

static void winner(struct etch_native_audio *a,uint8_t ch,uint8_t pitch,uint8_t vel){
 a->voice[0].active=1u;a->voice[0].held=1u;
 a->voice[0].note=(uint16_t)(128u+128u*ch+pitch);
 a->voice[0].velocity=vel;
}
int main(void){
 struct ru_midi_expression state={0};ru_midi_expr_reset(&state);
 struct etch_native_audio a={0};
 struct ru_touch_profile p={RU_TOUCH_REVERB,RU_TOUCH_MODE_VELOCITY,1u,0u,100u,1u};
 uint8_t source=55u,raw=55u;
 assert(!ru_midi_expr_held(&a));
 assert(!ru_midi_expr_level(&state,&a,&p,&source,&raw));
 assert(source==RU_MIDI_EXPR_NONE&&raw==0u);
 assert(ru_touch_profile_valid(&p));
 for(uint8_t feel=0u;feel<4u;feel++){
  assert(ru_midi_velocity_level(0u,feel)==0u);
  assert(ru_midi_velocity_level(1u,feel)==0u);
  assert(ru_midi_velocity_level(127u,feel)==127u);
  uint8_t last=0u;
  for(uint8_t v=1u;v<=127u;v++){
   uint8_t level=ru_midi_velocity_level(v,feel);
   assert(level>=last);last=level;
  }
 }
 assert(ru_midi_velocity_level(64u,0u)>ru_midi_velocity_level(64u,1u));
 assert(ru_midi_velocity_level(64u,2u)<ru_midi_velocity_level(64u,1u));
 assert(ru_midi_velocity_level(64u,3u)<ru_midi_velocity_level(64u,2u));
 winner(&a,2u,60u,64u);
 struct etch_native_audio unchanged=a;
 uint8_t level=ru_midi_expr_level(&state,&a,&p,&source,&raw);
 assert(source==RU_MIDI_EXPR_VELOCITY&&raw==64u);
 assert(level==ru_midi_velocity_level(64u,1u));
 assert(!memcmp(&unchanged,&a,sizeof(a))); /* no hidden MIDI note writes */
 p.mode=RU_TOUCH_MODE_PRESS;
 ru_midi_expr_event(&state,0xd2u,27u,0u); /* channel pressure */
 assert(ru_midi_expr_level(&state,&a,&p,&source,&raw)==27u);
 assert(source==RU_MIDI_EXPR_CHANNEL_PRESSURE&&raw==27u);
 ru_midi_expr_event(&state,0xa2u,60u,77u); /* poly AT wins */
 assert(ru_midi_expr_level(&state,&a,&p,&source,&raw)==77u);
 assert(source==RU_MIDI_EXPR_POLY_PRESSURE&&raw==77u);
 ru_midi_expr_event(&state,0xa2u,60u,0u); /* explicit zero must work */
 assert(!ru_midi_expr_level(&state,&a,&p,&source,&raw));
 assert(source==RU_MIDI_EXPR_POLY_PRESSURE&&raw==0u);
 p.mode=RU_TOUCH_MODE_VELOCITY; /* Note On velocity beats both AT sources */
 assert(ru_midi_expr_level(&state,&a,&p,&source,&raw)==level);
 assert(source==RU_MIDI_EXPR_VELOCITY&&raw==64u);
 p.mode=RU_TOUCH_MODE_PRESS;
 ru_midi_expr_event(&state,0x82u,60u,0u); /* clear only the poly tag */
 assert(ru_midi_expr_level(&state,&a,&p,&source,&raw)==27u);
 ru_midi_expr_event(&state,0xb2u,121u,0u); /* controller reset */
 assert(ru_midi_expr_level(&state,&a,&p,&source,&raw)==level);
 assert(source==RU_MIDI_EXPR_VELOCITY);
 ru_midi_expr_event(&state,0xa2u,60u,80u);
 ru_midi_expr_event(&state,0x92u,60u,12u); /* new strike may not inherit AT */
 assert(ru_midi_expr_level(&state,&a,&p,&source,&raw)==level);
 winner(&a,3u,60u,16u); /* never borrow other channel's pressure */
 assert(ru_midi_expr_level(&state,&a,&p,&source,&raw)==ru_midi_velocity_level(16u,1u));
 a.voice[0].held=0u;
 assert(!ru_midi_expr_level(&state,&a,&p,&source,&raw));
 p.target=RU_TOUCH_OFF;
 assert(ru_touch_profile_valid(&p)); /* independently preserved OFF mode */
 /* The REAL UART running-status parser preserves aftertouch alongside notes.
  * This is the same USB-style packet passed into the ALNK MIDI queue. */
 RuTrsParser trs={0};ru_trs_reset(&trs);
 uint32_t pkt=0u;
 assert(!ru_trs_feed(&trs,0xa0u,&pkt));
 assert(!ru_trs_feed(&trs,60u,&pkt));
 assert(ru_trs_feed(&trs,82u,&pkt));
 assert(((pkt>>8)&255u)==0xa0u&&((pkt>>16)&127u)==60u&&
  ((pkt>>24)&127u)==82u);
 assert(!ru_trs_feed(&trs,0xd0u,&pkt));
 assert(ru_trs_feed(&trs,63u,&pkt));
 assert(((pkt>>8)&255u)==0xd0u&&((pkt>>16)&127u)==63u);
 assert(!ru_trs_feed(&trs,0x90u,&pkt));
 assert(!ru_trs_feed(&trs,60u,&pkt));
 assert(ru_trs_feed(&trs,97u,&pkt));
 assert(((pkt>>8)&255u)==0x90u&&((pkt>>24)&127u)==97u);
 puts("PASS V2.0.58 MIDI velocity curves, poly/channel aftertouch, Note OFF and OFF target");
 return 0;
}
