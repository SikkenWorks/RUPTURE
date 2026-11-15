/* SPDX-License-Identifier: GPL-3.0-only
 * RUPTURE V1.99: GLIDE's UI-only train-tunnel playback gate.
 * No audio DSP, allocation, timer interrupt or flash access. */
#pragma once
#include <stdint.h>

static inline uint8_t ru_glide_tunnel_play_step(uint32_t now_ms,
    uint8_t note_held, uint8_t *frame, uint8_t *shake_tick,
    uint32_t *last_tick_ms, uint8_t frame_count){
 if(!frame_count)return 0u;
 if(!note_held){
  /* Silence means a fully stationary first image (including camera shake).
   * A fresh note starts the walk again after the usual 85 ms interval. */
  uint8_t changed=(uint8_t)(*frame!=0u || *shake_tick!=0u);
  *frame=0u;
  *shake_tick=0u;
  *last_tick_ms=now_ms;
  return changed;
 }
 if((uint32_t)(now_ms-*last_tick_ms)<85u)return 0u;
 *last_tick_ms=now_ms;
 if(++*shake_tick>=24u)*shake_tick=0u;
 if((*shake_tick%3u)==0u){
  if(++*frame>=frame_count)*frame=0u;
 }
 return 1u;
}
