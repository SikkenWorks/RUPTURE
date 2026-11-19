/* SPDX-License-Identifier: GPL-3.0-only
 * Bounded per-sample click guard around live PRESETS encoder changes.
 * Runs ONLY on the audio ISR; no buffer allocations, blocking or timer work.
 */
#ifndef ETCH_PRESET_RAMP_H
#define ETCH_PRESET_RAMP_H
#include <stdint.h>
#define ETCH_PRESET_RAMP_SAMPLES 256u
struct etch_preset_ramp {
 int32_t last_l,last_r,hold_l,hold_r;
 uint16_t remaining;
};
static inline void etch_preset_ramp_reset(struct etch_preset_ramp *r){
 r->last_l=0;r->last_r=0;r->hold_l=0;r->hold_r=0;r->remaining=0;
}
static inline void etch_preset_ramp_begin(struct etch_preset_ramp *r){
 r->hold_l=r->last_l;r->hold_r=r->last_r;
 r->remaining=ETCH_PRESET_RAMP_SAMPLES;
}
/* Truncate signed Q8 towards zero, exactly as the historical /256 did.
 * Keep division out of the DMA ISR even on compilers without constant
 * division strength reduction. Bounds: |x| <= 32768*256. */
static inline int32_t etch_preset_ramp_q8(int32_t x){
 return x>=0?x>>8:-((-x)>>8);
}
static inline void etch_preset_ramp_frame(struct etch_preset_ramp *r,int32_t *l,int32_t *right){
 if(r->remaining){
  uint32_t fade_in=(uint32_t)(ETCH_PRESET_RAMP_SAMPLES-r->remaining);
  uint32_t fade_out=(uint32_t)r->remaining;
  *l=etch_preset_ramp_q8(r->hold_l*(int32_t)fade_out+(*l)*(int32_t)fade_in);
  *right=etch_preset_ramp_q8(r->hold_r*(int32_t)fade_out+(*right)*(int32_t)fade_in);
  --r->remaining;
 }
 r->last_l=*l;r->last_r=*right;
}
#endif
