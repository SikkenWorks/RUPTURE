/* RUPTURE V1.63: one gesture of safe UI-only encoder undo.
 * No audio callback, no allocation, no flash writes.
 * SPDX-License-Identifier: GPL-3.0-only */
#ifndef RUPTURE_UNDO_GESTURE_H
#define RUPTURE_UNDO_GESTURE_H
#include <stdint.h>
typedef struct {uint32_t at_ms;uint8_t valid,page,control,old_value;} RuKnobUndo;
static inline void ru_undo_gesture_capture(RuKnobUndo *u,uint8_t page,uint8_t control,
                                           uint8_t before,uint32_t now){
 if(!u->valid || u->page!=page || u->control!=control ||
    (uint32_t)(now-u->at_ms)>650u){
  u->old_value=before;u->page=page;u->control=control;u->valid=1u;
 }
 u->at_ms=now;
}
#endif
