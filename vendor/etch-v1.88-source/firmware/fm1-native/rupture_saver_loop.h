/* SPDX-License-Identifier: GPL-3.0-only
 * RUPTURE V1.45: no extra framebuffer / animation frames.
 * Frame 0..63 forward, hold final 2200 ms, 63..0 backwards, repeat.
 * Called solely from UI/main loop at 150ms intervals.
 */
#ifndef RU_SAVER_LOOP_H
#define RU_SAVER_LOOP_H
#include <stdint.h>
static inline uint8_t ru_saver_step(uint8_t *frame,uint8_t *reverse,
                                   uint32_t *hold_until,uint32_t now){
 if(*hold_until){
  if((int32_t)(now-*hold_until)>=0){*hold_until=0u;*reverse=1u;}
  return 0u;
 }
 if(*reverse){
  if(*frame){--*frame;if(!*frame)*reverse=0u;return 1u;}
  *reverse=0u;
 }
 if(*frame<63u){
  ++*frame;
  if(*frame==63u)*hold_until=now+2200u;
  return 1u;
 }
 *hold_until=now+2200u;
 return 0u;
}
#endif
