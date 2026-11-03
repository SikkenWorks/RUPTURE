/* RUPTURE V1.56: lightweight ENV/LFO note-gated UI animation.
 * Never called from audio ISR. Frame indexes and timestamps only.
 * LFO: 0..12 open on KEY DOWN, reverse exactly 12..0 on KEY UP.
 * ENV: 0..12 attack/hold, then 13..19 release/close.
 * SHAPE: 0..10 erupt, 11..13 3-frame lava loop, 14..19 cool/close.
 * SPDX-License-Identifier: GPL-3.0-only */
#ifndef RUPTURE_PANEL_ANIM_H
#define RUPTURE_PANEL_ANIM_H
#include <stdint.h>
typedef struct {uint32_t next_ms;uint8_t frame;uint8_t phase;} RuPanelAnim;
enum {RU_PANEL_IDLE=0u,RU_PANEL_OPEN=1u,RU_PANEL_HOLD=2u,RU_PANEL_CLOSE=3u};
static uint8_t ru_panel_update(RuPanelAnim *a,uint32_t now,uint8_t gates,uint8_t mode){
 uint8_t prev=a->frame;
 if(gates){
  if(a->phase==RU_PANEL_IDLE){a->phase=RU_PANEL_OPEN;a->frame=1u;a->next_ms=now+80u;}
  else if(a->phase==RU_PANEL_CLOSE){a->phase=RU_PANEL_OPEN;if(mode==2u)a->frame=1u;a->next_ms=now+80u;}
  else if(mode==2u && a->phase==RU_PANEL_HOLD && (int32_t)(now-a->next_ms)>=0){
   a->frame=a->frame>=13u?11u:(uint8_t)(a->frame+1u);a->next_ms=now+95u;
  }
  else if(a->phase==RU_PANEL_OPEN && (int32_t)(now-a->next_ms)>=0){
   if(a->frame<(mode==2u?10u:12u)){++a->frame;a->next_ms=now+80u;}
   else if(mode==2u){a->phase=RU_PANEL_HOLD;a->frame=11u;a->next_ms=now+95u;}
   else{a->phase=RU_PANEL_HOLD;a->next_ms=0u;}
  }
 }else{
  if(a->phase==RU_PANEL_OPEN || a->phase==RU_PANEL_HOLD){
   a->phase=RU_PANEL_CLOSE;
   if(mode!=0u){
    /* No abrupt jump into a fully collapsed wall after a short key press. */
    a->frame=(uint8_t)(mode==2u ?
       (a->frame<=2u?18u:a->frame<=4u?17u:a->frame<=6u?16u:a->frame<=8u?15u:14u) :
       (a->frame<=2u?18u:a->frame<=4u?17u:a->frame<=6u?16u:a->frame<=8u?15u:a->frame<=10u?14u:13u));
   }
   a->next_ms=now+(mode==0u?80u:90u);
  }else if(a->phase==RU_PANEL_CLOSE && (int32_t)(now-a->next_ms)>=0){
   if(mode!=0u){
    if(a->frame<19u){++a->frame;a->next_ms=now+90u;}
    else{a->frame=0u;a->phase=RU_PANEL_IDLE;a->next_ms=0u;}
   }else{
    if(a->frame>0u){--a->frame;a->next_ms=now+80u;
     if(a->frame==0u){a->phase=RU_PANEL_IDLE;a->next_ms=0u;}}
    else{a->phase=RU_PANEL_IDLE;a->next_ms=0u;}
   }
  }
 }
 return prev!=a->frame;
}
#endif
