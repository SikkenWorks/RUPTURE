/* RUPTURE DAM: tiny display-only state machine (no audio ISR work).
 * Source sprites in rupture_dam_sprites.h are NOT generated procedurally.
 * SPDX-License-Identifier: GPL-3.0-only
 */
#ifndef RUPTURE_DAM_ANIM_H
#define RUPTURE_DAM_ANIM_H
#include <stdint.h>
enum {RU_DAM_IDLE=0u,RU_DAM_OPEN=1u,RU_DAM_LEAK=2u,RU_DAM_CLOSE=3u};
typedef struct {uint32_t next_ms;uint8_t frame;uint8_t phase;} RuDamAnim;
/* Display octave offsets: -2 through +5, clamped beyond the known range. */
static uint16_t ru_dam_leak_interval(int8_t octave){
 static const uint16_t speed[8]={270u,220u,165u,115u,75u,55u,45u,35u};
 if(octave < -2)octave=-2;
 if(octave > 5)octave=5;
 return speed[(uint8_t)(octave+2)];
}
static uint8_t ru_dam_update(RuDamAnim *a,uint32_t now,uint8_t held,int8_t octave){
 uint8_t prev=a->frame;
 if(held){
  if(a->phase==RU_DAM_IDLE || a->phase==RU_DAM_CLOSE){
   a->phase=RU_DAM_OPEN;a->frame=1u;a->next_ms=now+65u;
  }else if((int32_t)(now-a->next_ms)>=0){
   if(a->phase==RU_DAM_OPEN){
    if(a->frame<4u){a->frame++;a->next_ms=now+65u;}
    else {a->phase=RU_DAM_LEAK;a->frame=5u;a->next_ms=now+ru_dam_leak_interval(octave);}
   }else if(a->phase==RU_DAM_LEAK){
    a->frame=(uint8_t)(a->frame==7u?5u:a->frame+1u);
    a->next_ms=now+ru_dam_leak_interval(octave);
   }
  }
 }else{
  if(a->phase==RU_DAM_OPEN || a->phase==RU_DAM_LEAK){
   /* Immediately stop liquid on the LAST released key (including sequencer/MIDI).
    * Choose closing frame based on how far the crack had already opened. */
   uint8_t last=a->frame;
   a->phase=RU_DAM_CLOSE;
   a->frame=last<=1u?10u:last==2u?9u:8u;
   a->next_ms=now+75u;
  }else if(a->phase==RU_DAM_CLOSE && (int32_t)(now-a->next_ms)>=0){
   if(a->frame<11u){a->frame++;a->next_ms=now+75u;}
   else {a->frame=0u;a->phase=RU_DAM_IDLE;a->next_ms=0u;}
  }
 }
 return (uint8_t)(prev!=a->frame);
}
#endif
