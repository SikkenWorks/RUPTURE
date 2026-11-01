/* RUPTURE hardware SAVE destination routing.
 * Used by the FM1 input loop and host regression: any of the four HOME
 * knob pages opens the same named project browser, never a direct flash write.
 * SPDX-License-Identifier: GPL-3.0-only */
#ifndef RUPTURE_SAVE_ROUTE_H
#define RUPTURE_SAVE_ROUTE_H
#include <stdint.h>
enum { RU_SAVE_NO_MENU=0u, RU_SAVE_PROJECT_MENU=1u, RU_SAVE_SEQUENCE_MENU=2u };
static inline uint8_t ru_save_destination(uint8_t page){
 switch(page){
 case RU_PAGE_PLAY:
 case RU_PAGE_SHAPE:
 case RU_PAGE_SUB1:
 case RU_PAGE_SUB2:
 case RU_PAGE_PROJECTS:
  return RU_SAVE_PROJECT_MENU;
 case RU_PAGE_SEQ:
 case RU_PAGE_SEQ_SLOTS:
  return RU_SAVE_SEQUENCE_MENU;
 default:
  return RU_SAVE_NO_MENU;
 }
}
/* SAVE + FX chord stays reserved; no hidden project actions on a chord. */
static inline uint8_t ru_save_pressed(uint32_t pressed,uint32_t held){
 return (uint8_t)((pressed&(1u<<9))!=0u && (held&(1u<<2))==0u);
}
#endif
