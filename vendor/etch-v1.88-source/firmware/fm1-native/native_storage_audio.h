/* SPDX-License-Identifier: GPL-3.0-only
 * Foreground storage handoff. The ISR fades PCM, then overwrites BOTH
 * DMA halves with zero before foreground may borrow the flash controller.
 * No clock/analogue enable changes belong to a successful handoff.
 */
#pragma once
#include <stdint.h>
#define ETCH_STORAGE_FADE_FRAMES 256u
#define ETCH_STORAGE_QUIET_TIMEOUT_TICKS 1200000u /* 50 ms at 24 MHz */
struct etch_storage_audio {
 uint16_t remaining;
 uint8_t zero_halves;
};
static inline void etch_storage_audio_begin(volatile struct etch_storage_audio *s){
 s->remaining=ETCH_STORAGE_FADE_FRAMES;s->zero_halves=0u;
}
static inline void etch_storage_audio_frame(volatile struct etch_storage_audio *s,
 int32_t *left,int32_t *right){
 if(s->remaining){
  --s->remaining;
  *left=(*left*(int32_t)s->remaining)/(int32_t)ETCH_STORAGE_FADE_FRAMES;
  *right=(*right*(int32_t)s->remaining)/(int32_t)ETCH_STORAGE_FADE_FRAMES;
 }else{*left=0;*right=0;}
}
/* Call ONLY after an entirely zero block has passed both DMA ownership
 * checks. A partly faded block does not count, nor does a rejected copy. */
static inline void etch_storage_audio_published(volatile struct etch_storage_audio *s,
 uint8_t entirely_zero,uint32_t half){
 if(entirely_zero&&half<2u)s->zero_halves|=(uint8_t)(1u<<half);
}
static inline uint8_t etch_storage_audio_ready(const volatile struct etch_storage_audio *s){
 return (uint8_t)(!s->remaining&&s->zero_halves==3u);
}
