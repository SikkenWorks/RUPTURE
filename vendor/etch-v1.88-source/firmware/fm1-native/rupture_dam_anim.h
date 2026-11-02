/* RUPTURE DAM V3: event-driven UI-only animation state; never audio ISR.
 * 20 original bitmap frames: 0..9 open, 10..12 flow loop, 13..19 close.
 * SPDX-License-Identifier: GPL-3.0-only
 */
#ifndef RUPTURE_DAM_ANIM_H
#define RUPTURE_DAM_ANIM_H
#include <stdint.h>
enum {RU_DAM_IDLE=0u,RU_DAM_OPEN=1u,RU_DAM_LEAK=2u,RU_DAM_CLOSE=3u};
typedef struct {uint32_t next_ms;uint8_t frame;uint8_t phase;} RuDamAnim;
static uint16_t ru_dam_leak_interval(int8_t octave){
 static const uint16_t speed[8]={250u,200u,140u,110u,85u,65u,50u,40u};
 if(octave < -2)octave=-2;
 if(octave > 5)octave=5;
 return speed[(uint8_t)(octave+2)];
}
static uint8_t ru_dam_update(RuDamAnim *a,uint32_t now,uint8_t held,int8_t octave){
 uint8_t previous=a->frame;
 if(held){
  if(a->phase==RU_DAM_IDLE||a->phase==RU_DAM_CLOSE){
   a->phase=RU_DAM_OPEN;a->frame=1u;a->next_ms=now+65u;
  }else if((int32_t)(now-a->next_ms)>=0){
   if(a->phase==RU_DAM_OPEN){
    if(a->frame<9u){++a->frame;a->next_ms=now+65u;}
    else{a->phase=RU_DAM_LEAK;a->frame=10u;a->next_ms=now+ru_dam_leak_interval(octave);}
   }else if(a->phase==RU_DAM_LEAK){
    a->frame=(uint8_t)(a->frame==12u?10u:a->frame+1u);
    a->next_ms=now+ru_dam_leak_interval(octave);
   }
  }
 }else{
  if(a->phase==RU_DAM_OPEN||a->phase==RU_DAM_LEAK){
   /* On release, stop the moving 3-frame stream and transition to recession.
    * A short key press must never jump forward into maximum flooding. */
   uint8_t last=a->frame;
   a->phase=RU_DAM_CLOSE;
   a->frame=(uint8_t)(last<=2u?18u:last<=4u?17u:last<=6u?16u:last<=8u?15u:13u);
   a->next_ms=now+75u;
  }else if(a->phase==RU_DAM_CLOSE&&(int32_t)(now-a->next_ms)>=0){
   if(a->frame<19u){++a->frame;a->next_ms=now+75u;}
   else{a->frame=0u;a->phase=RU_DAM_IDLE;a->next_ms=0u;}
  }
 }
 return (uint8_t)(previous!=a->frame);
}
#endif
