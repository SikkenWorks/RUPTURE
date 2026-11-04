/* RUPTURE V1.47: active SAW PLAY and SOUND SHAPING controls.
 * No heap, extra oscillator, wavetable, floating point or variable divide.
 * Shared legacy CXS macro bytes persist all four values unchanged.
 * SPDX-License-Identifier: GPL-3.0-only */
#ifndef RUPTURE_SAW_TONE_V176_H
#define RUPTURE_SAW_TONE_V176_H
#include <stdint.h>
/* One bounded one-pole state; 32-bit products remain below INT32_MAX.
 * PLAY: BRIGHT (tilt), EDGE (drive), BODY (bass), LEVEL (gain).
 * SOUND SHAPING: WEIGHT (low boost), EDGE (colour), BITE (odd saturation),
 * PUNCH (saw-edge attack, audible also during sustained notes).
 * The following path runs only when the selected carrier has SAW content. */
static inline int32_t rupture_saw_tone_v176(int32_t *low_state,int32_t raw,
 uint8_t bright,uint8_t edge,uint8_t body,uint8_t level,
 uint8_t shape_edge,uint8_t shape_weight,uint8_t shape_bite,uint8_t shape_punch){
 if(bright>100u)bright=100u;
 if(edge>100u)edge=100u;
 if(body>100u)body=100u;
 if(level>100u)level=100u;
 if(shape_edge>100u)shape_edge=100u;
 if(shape_weight>100u)shape_weight=100u;
 if(shape_bite>100u)shape_bite=100u;
 if(shape_punch>100u)shape_punch=100u;
 /* Approx 1.4 kHz one-pole at 44.1 kHz, no per-sample division. */
 int32_t low=*low_state + ((raw-*low_state)>>3);
 *low_state=low;
 int32_t wet=low+(((raw-low)*((int32_t)bright*5/2))>>8);
 wet+=(low*(int32_t)body)>>9;
 /* EDGE neutral at 50, preserves both polarities and soft-knee headroom. */
 int32_t drive=256+((int32_t)edge-50)*3;
 wet=(wet*drive)>>8;
 if(wet>24576)wet=24576+((wet-24576)>>3);
 if(wet< -24576)wet=-24576-(((-wet)-24576)>>3);
 if(wet>32767)wet=32767;
 if(wet< -32767)wet=-32767;
 /* SOUND SHAPE EDGE has an audible SAW mapping too. Neutral at 50.
  * Only a bounded multiply; CXS still uses its unmodified edge engine. */
 if(shape_edge!=50u){
  wet=(wet*(256+((int32_t)shape_edge-50)*2))>>8;
  if(wet>32767)wet=32767;
  if(wet< -32767)wet=-32767;
 }
 /* LEVEL default 68 ~= unity; can trim down or modestly boost up. */
 wet=(wet*(16384+(int32_t)level*241))>>15;
 if(wet>32767)wet=32767;
 if(wet< -32767)wet=-32767;
 /* SOUND SHAPING is independent of the CXS shaper. Unlike the original
  * CXS PUNCH, SAW PUNCH must remain audible after eight oscillator cycles.
  * Reuse the already calculated low-pass state: zero extra filter states,
  * no 64-bit math, division, tables, allocation or further wavetable reads.
  * Inactive controls preserve V1.46 PCM exactly. */
 if(shape_weight){
  wet+=(low*(int32_t)shape_weight)>>7;
  if(wet>32767)wet=32767;
  if(wet< -32767)wet=-32767;
 }
 if(shape_bite){
  int32_t magnitude=wet<0?-wet:wet;
  int32_t harmonic=(wet*(32767-magnitude))>>15;
  wet+=(harmonic*(int32_t)shape_bite)>>7;
  if(wet>32767)wet=32767;
  if(wet< -32767)wet=-32767;
 }
 if(shape_punch){
  /* High-pass residual accents each rising edge, not just note-on. */
  int32_t residual=raw-low;
  wet+=(residual*(int32_t)shape_punch)>>8;
 }
 if(wet>32767)return 32767;
 if(wet< -32767)return -32767;
 return wet;
}
#endif
