/* RUPTURE V1.48 physical SAVE routing.
 * Any ordinary UI page invokes the PROJECTS browser. The sequencer and its
 * SEQ SLOTS screen retain the independent sequence library; the NAME editor
 * retains SAVE-to-commit. Boot, transfer and SAFE MODE are not editable.
 * No flash IO is performed by entering the browser.
 * SPDX-License-Identifier: GPL-3.0-only */
#ifndef RUPTURE_SAVE_ROUTE_H
#define RUPTURE_SAVE_ROUTE_H
#include <stdint.h>
enum { RU_SAVE_NO_MENU=0u, RU_SAVE_PROJECT_MENU=1u, RU_SAVE_SEQUENCE_MENU=2u };
static inline uint8_t ru_save_destination(uint8_t page){
 switch(page){
 case RU_PAGE_SEQ:
 case RU_PAGE_SEQ_SLOTS:
  return RU_SAVE_SEQUENCE_MENU;
 case RU_PAGE_NAME: /* SAVE commits the current project/sequence name. */
 case RU_PAGE_UPDATE:
 case RU_PAGE_FOREIGN:
 case RU_PAGE_FORMAT_CONFIRM:
 case RU_PAGE_SAFE:
 case RU_PAGE_LOADING:
 case RU_PAGE_SYS_INFO:
 case RU_PAGE_TRANSIENT: /* Unused, deliberately disabled legacy views. */
 case RU_PAGE_BEND:
  return RU_SAVE_NO_MENU;
 default:
  return page<RU_PAGE_COUNT?RU_SAVE_PROJECT_MENU:RU_SAVE_NO_MENU;
 }
}
/* SAVE+FX chord is reserved. Explicit SEL+SAVE rename in PROJECTS/SEQ SLOTS
 * is handled separately by the UI route, never intercepted here. */
static inline uint8_t ru_save_pressed(uint32_t pressed,uint32_t held){
 return (uint8_t)((pressed&(1u<<9))!=0u && (held&(1u<<2))==0u);
}
#endif
