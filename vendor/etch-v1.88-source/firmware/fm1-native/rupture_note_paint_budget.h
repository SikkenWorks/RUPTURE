/* SPDX-License-Identifier: GPL-3.0-only
 * RUPTURE V1.81: note-burst LCD arbitration, MAIN-LOOP ONLY.
 * Audio and all input events remain full-rate; only repeated visual
 * redraw requests are coalesced while several notes arrive rapidly.
 */
#ifndef RUPTURE_NOTE_PAINT_BUDGET_H
#define RUPTURE_NOTE_PAINT_BUDGET_H
#include <stdint.h>
struct ru_note_paint_budget {
 uint32_t last_note_ms;
 uint8_t burst;
 uint8_t pending;
};
static inline void ru_note_paint_changed(struct ru_note_paint_budget *s,uint32_t now){
 if(!s)return;
 if(s->pending && (uint32_t)(now-s->last_note_ms)<=125u){
  if(s->burst<255u)++s->burst;
 }else if((uint32_t)(now-s->last_note_ms)<=125u && s->burst){
  if(s->burst<255u)++s->burst;
 }else s->burst=1u;
 s->last_note_ms=now;s->pending=1u;
}
static inline uint8_t ru_note_paint_allow(const struct ru_note_paint_budget *s,
 uint32_t now,uint32_t last_paint,uint8_t user_control,uint8_t audio_pressure){
 if(user_control || !s || s->burst<3u)return 1u;
 /* Two or more very recent note edges are already pending. At 40 FPS,
  * repeated full-screen SPI transactions contend with ALNK DMA. */
 uint16_t floor_ms=audio_pressure>=12u?100u:75u;
 if((uint32_t)(now-s->last_note_ms)>150u)return 1u;
 return (uint8_t)((uint32_t)(now-last_paint)>=floor_ms);
}
static inline void ru_note_paint_committed(struct ru_note_paint_budget *s){if(s)s->pending=0u;}
#endif
