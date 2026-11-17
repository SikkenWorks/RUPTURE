/* RUPTURE V2.0.3: SUB OSC 1 holds its forward stare; SUB OSC 2 unchanged. */
#ifndef RUPTURE_SUBOSC_FEAR_PLAYBACK_H
#define RUPTURE_SUBOSC_FEAR_PLAYBACK_H
#include <stdint.h>

#define RU_SUB_FEAR_TICK_MS 75u
#define RU_SUB_FEAR_LOOP_FIRST 12u
#define RU_SUB_FEAR_LOOP_LAST 15u
#define RU_SUB1_STARE_FRAME 11u

static inline uint8_t ru_sub_fear_play_step(uint32_t now_ms,uint8_t held,
    uint8_t *frame,uint8_t *active,uint32_t *last_ms){
 if(!held){
  uint8_t changed=(uint8_t)(*active||*frame);
  *active=0u;*frame=0u;*last_ms=now_ms;
  return changed;
 }
 if(!*active){
  *active=1u;*frame=1u;*last_ms=now_ms;
  return 1u;
 }
 if((uint32_t)(now_ms-*last_ms)<RU_SUB_FEAR_TICK_MS)return 0u;
 *last_ms=now_ms;
 if(*frame<RU_SUB_FEAR_LOOP_LAST)(*frame)++;
 else *frame=RU_SUB_FEAR_LOOP_FIRST;
 return 1u;
}

/* SUB OSC 1: turn once, then keep exactly frame 12 on screen.
 * Small whole-camera offsets provide the tremble, not cycling facial art.
 * The playback cadence and rendering are UI-only; no audio processing. */
static inline uint8_t ru_sub1_stare_step(uint32_t now_ms,uint8_t held,
    uint8_t *frame,uint8_t *active,uint32_t *last_ms,uint8_t *shake){
 if(!held){
  uint8_t changed=(uint8_t)(*active||*frame||*shake);
  *active=0u;*frame=0u;*shake=0u;*last_ms=now_ms;
  return changed;
 }
 if(!*active){
  *active=1u;*frame=1u;*shake=0u;*last_ms=now_ms;
  return 1u;
 }
 if((uint32_t)(now_ms-*last_ms)<RU_SUB_FEAR_TICK_MS)return 0u;
 *last_ms=now_ms;
 if(*frame<RU_SUB1_STARE_FRAME)(*frame)++;
 else *shake=(uint8_t)((*shake+1u)&7u);
 return 1u;
}
#endif
