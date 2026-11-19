/* SPDX-License-Identifier: GPL-3.0-only
 * RUPTURE V1.81: note-burst LCD arbitration, MAIN-LOOP ONLY.
 * Audio and all input events remain full-rate; only repeated visual
 * redraw requests are coalesced while several notes arrive rapidly.
 */
#ifndef RUPTURE_NOTE_PAINT_BUDGET_H
#define RUPTURE_NOTE_PAINT_BUDGET_H
#include <stdint.h>
#include "rupture_fx_chain.h"
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
/* V2.0.37: a SINGLE note-on/off can overrun the ALNK window when the
 * screen's SPI DMA also fetches FX code from XIP. The old burst gate only
 * activated after three notes. Count real, enabled ordered FX slots once
 * per UI tick, and postpone purely cosmetic LCD DMA for 64ms around EACH
 * note edge with three or more active effects. Audio, note priority, FX
 * processing, saves, user commands and fault screens are never delayed. */
static inline uint8_t ru_note_fx_active_count(const volatile struct rupture_fx_chain *c,
 uint8_t active){
 if(!active||!c)return 0u;
 uint8_t count=0u;
 for(uint8_t i=0u;i<RU_FX_SLOTS;i++)
  if((c->enabled&(1u<<i))&&c->type[i]!=RU_FX_EMPTY)++count;
 return count;
}
static inline uint8_t ru_note_paint_allow_fx(const struct ru_note_paint_budget *s,
 uint32_t now,uint32_t last_paint,uint8_t user_control,
 uint8_t audio_pressure,uint8_t active_fx){
 if(!user_control&&s&&s->pending&&active_fx>=3u&&
    (uint32_t)(now-s->last_note_ms)<64u)return 0u;
 return ru_note_paint_allow(s,now,last_paint,user_control,audio_pressure);
}
/* V2.0.38: optional waveform/CRT animation cannot fetch LCD SPI buffers
 * 25 times a second during a sustained 3+ FX bass note. Allow at most one
 * cosmetic frame per 120 ms while a note sounds, while retaining the 64 ms
 * note-edge quiet window and never postponing an explicit UI command. */
static inline uint8_t ru_note_paint_allow_fx_sustain(
 const struct ru_note_paint_budget *s,uint32_t now,uint32_t last_paint,
 uint8_t user_control,uint8_t audio_pressure,uint8_t active_fx,
 uint8_t sounding){
 if(!user_control&&active_fx>=3u&&sounding&&
    (uint32_t)(now-last_paint)<120u)return 0u;
 return ru_note_paint_allow_fx(s,now,last_paint,user_control,
                               audio_pressure,active_fx);
}
static inline void ru_note_paint_committed(struct ru_note_paint_budget *s){if(s)s->pending=0u;}
#endif
