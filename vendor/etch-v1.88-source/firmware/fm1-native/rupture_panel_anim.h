/* RUPTURE ENV wall-break, LFO sinkhole and SOUND SHAPE volcano.
 * Opening/closing sprites from V1.56/V1.57 remain byte-identical.
 * V1.72: ENV smoke/debris settle once, then the cracked wall trembles subtly; LFO debris
 * falls in an endless eight-frame loop. Pure UI; never runs in audio ISR.
 * SPDX-License-Identifier: GPL-3.0-only */
#ifndef RUPTURE_PANEL_ANIM_H
#define RUPTURE_PANEL_ANIM_H
#include <stdint.h>
#define RU_PANEL_HOLD_FRAMES 8u
typedef struct {
 uint32_t next_ms;
 uint8_t frame,phase,shake,hold_frame;
} RuPanelAnim;
enum {RU_PANEL_IDLE=0u,RU_PANEL_OPEN=1u,RU_PANEL_HOLD=2u,RU_PANEL_CLOSE=3u};
static uint8_t ru_panel_update(RuPanelAnim *a,uint32_t now,uint8_t gates,uint8_t mode){
 uint8_t prev=a->frame,prev_shake=a->shake,prev_hold=a->hold_frame,prev_phase=a->phase;
 if(gates){
  if(a->phase==RU_PANEL_IDLE){
   a->phase=RU_PANEL_OPEN;a->frame=1u;a->shake=0u;a->hold_frame=0u;a->next_ms=now+80u;
  }else if(a->phase==RU_PANEL_CLOSE){
   /* Retrigger during a closing animation resumes the existing opening
    * without ever indexing outside its 0..19 original art range. */
   if(mode==2u){a->phase=RU_PANEL_OPEN;a->frame=1u;}
   else if(mode==1u && a->frame>12u){
    a->phase=RU_PANEL_HOLD;a->frame=12u;
   }else{a->phase=RU_PANEL_OPEN;if(a->frame>12u)a->frame=12u;}
   a->hold_frame=0u;a->shake=0u;a->next_ms=now+80u;
  }else if(a->next_ms){
   for(uint8_t i=0u;i<32u && (int32_t)(now-a->next_ms)>=0;i++){
    if(a->phase==RU_PANEL_HOLD){
     if(mode==2u){
      a->frame=a->frame>=13u?11u:(uint8_t)(a->frame+1u);a->next_ms+=95u;
     }else if(mode==0u){
      /* New dirt pebbles descend forever: 8 ROM frames, no per-pixel DSP. */
      a->hold_frame=(uint8_t)((a->hold_frame+1u)%RU_PANEL_HOLD_FRAMES);
      a->shake=(uint8_t)((a->shake+1u)%3u);
      a->next_ms+=115u;
     }else{
      /* Eight one-way PNG frames: smoke and falling fragments settle.
       * Once frame eight is reached, do NOT loop the smoke. Only the
       * established <=1 px UI blit trembles while the note remains held. */
      if(a->hold_frame<RU_PANEL_HOLD_FRAMES-1u){
       ++a->hold_frame;
       a->shake=(uint8_t)((a->shake+1u)%3u);
       a->next_ms+=150u;
      }else{
       a->shake=(uint8_t)((a->shake+1u)%3u);
       a->next_ms+=220u;
      }
     }
    }else if(a->phase==RU_PANEL_OPEN){
     if(a->frame<(mode==2u?10u:12u)){
      ++a->frame;a->next_ms+=80u;
     }else if(mode==2u){
      a->phase=RU_PANEL_HOLD;a->frame=11u;a->next_ms+=95u;
     }else{
      a->phase=RU_PANEL_HOLD;a->hold_frame=0u;a->shake=0u;
      a->next_ms+=mode==0u?115u:150u;
     }
    }else break;
   }
   if(a->next_ms && (int32_t)(now-a->next_ms)>=0)
    a->next_ms=now+(mode==2u?95u:mode==0u?115u:150u);
  }
 }else{
  if(a->phase==RU_PANEL_OPEN || a->phase==RU_PANEL_HOLD){
   a->phase=RU_PANEL_CLOSE;a->shake=0u;a->hold_frame=0u;
   if(mode!=0u){
    /* No sudden jump to a fully closed wall after a short press. */
    a->frame=(uint8_t)(mode==2u ?
       (a->frame<=2u?18u:a->frame<=4u?17u:a->frame<=6u?16u:a->frame<=8u?15u:14u) :
       (a->frame<=2u?18u:a->frame<=4u?17u:a->frame<=6u?16u:a->frame<=8u?15u:a->frame<=10u?14u:13u));
   }
   a->next_ms=now+(mode==0u?80u:90u);
  }else if(a->phase==RU_PANEL_CLOSE){
   for(uint8_t i=0u;i<20u && (int32_t)(now-a->next_ms)>=0;i++){
    if(mode!=0u){
     if(a->frame<19u){++a->frame;a->next_ms+=90u;}
     else{a->frame=0u;a->phase=RU_PANEL_IDLE;a->next_ms=0u;break;}
    }else{
     if(a->frame>0u){--a->frame;a->next_ms+=80u;
      if(a->frame==0u){a->phase=RU_PANEL_IDLE;a->next_ms=0u;break;}}
     else{a->phase=RU_PANEL_IDLE;a->next_ms=0u;break;}
    }
   }
   if(a->phase==RU_PANEL_CLOSE && (int32_t)(now-a->next_ms)>=0)
    a->next_ms=now+(mode==0u?80u:90u);
  }
 }
 return (uint8_t)(prev!=a->frame || prev_shake!=a->shake ||
                  prev_hold!=a->hold_frame || prev_phase!=a->phase);
}
#endif
