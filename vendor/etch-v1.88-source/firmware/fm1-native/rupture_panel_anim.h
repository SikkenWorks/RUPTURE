/* ENV/LFO wall cracking and SOUND SHAPE volcano, original 20 sprites.
 * LFO: 0..12 open, 12..0 close. ENV: 0..12 open, 13..19 close.
 * SHAPE: 0..10 open, 11..13 three-frame eruption, 14..19 cool.
 * V1.66: wall-clock deadline catch-up, independent of actual LCD paints.
 * UI main loop only. SPDX-License-Identifier: GPL-3.0-only */
#ifndef RUPTURE_PANEL_ANIM_H
#define RUPTURE_PANEL_ANIM_H
#include <stdint.h>
typedef struct {uint32_t next_ms;uint8_t frame;uint8_t phase;uint8_t shake;} RuPanelAnim;
enum {RU_PANEL_IDLE=0u,RU_PANEL_OPEN=1u,RU_PANEL_HOLD=2u,RU_PANEL_CLOSE=3u};
static uint8_t ru_panel_update(RuPanelAnim *a,uint32_t now,uint8_t gates,uint8_t mode){
 uint8_t prev=a->frame,prev_shake=a->shake;
 if(gates){
  if(a->phase==RU_PANEL_IDLE){a->phase=RU_PANEL_OPEN;a->frame=1u;a->next_ms=now+80u;}
  else if(a->phase==RU_PANEL_CLOSE){
   a->phase=RU_PANEL_OPEN;if(mode==2u)a->frame=1u;a->next_ms=now+80u;
  }else{
   for(uint8_t i=0u;i<32u && (int32_t)(now-a->next_ms)>=0;i++){
    if(a->phase==RU_PANEL_HOLD){
     if(mode==2u){a->frame=a->frame>=13u?11u:(uint8_t)(a->frame+1u);a->next_ms+=95u;}
     else{a->shake=(uint8_t)((a->shake+1u)%3u);a->next_ms+=110u;}
    }else if(a->phase==RU_PANEL_OPEN){
     if(a->frame<(mode==2u?10u:12u)){++a->frame;a->next_ms+=80u;}
     else if(mode==2u){a->phase=RU_PANEL_HOLD;a->frame=11u;a->next_ms+=95u;}
     else{a->phase=RU_PANEL_HOLD;a->shake=0u;a->next_ms+=110u;}
    }else break;
   }
   if((int32_t)(now-a->next_ms)>=0)a->next_ms=now+(mode==2u?95u:110u);
  }
 }else{
  if(a->phase==RU_PANEL_OPEN || a->phase==RU_PANEL_HOLD){
   a->phase=RU_PANEL_CLOSE;a->shake=0u;
   if(mode!=0u){
    /* Never jump to the fully collapsed wall on a short key press. */
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
   if(a->phase==RU_PANEL_CLOSE && (int32_t)(now-a->next_ms)>=0)a->next_ms=now+(mode==0u?80u:90u);
  }
 }
 return (uint8_t)(prev!=a->frame||prev_shake!=a->shake);
}
#endif
