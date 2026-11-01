/* RUPTURE: optional four-parameter post-CXS sound shaper.
 * Pure deterministic fixed-point DSP; zeroed controls preserve baseline PCM.
 * SPDX-License-Identifier: GPL-3.0-only */
#ifndef RUPTURE_SHAPE_H
#define RUPTURE_SHAPE_H
#include <stdint.h>
static inline int32_t rupture_shape_clamp(int64_t x){
 return (int32_t)(x>32767?32767:x< -32767?-32767:x);
}
static inline int32_t rupture_shape_sample(int32_t *body_low,int32_t sample,
     uint8_t weight,uint8_t bite,uint8_t punch,uint32_t cycles){
 /* 220 Hz approx at 44.1kHz. Inactive WEIGHT is bit-exact bypass. */
 *body_low+=(sample-*body_low)/32;
 if(weight)sample=rupture_shape_clamp((int64_t)sample+
            (((int64_t)*body_low*weight)>>7));
 /* Odd-order soft saturation: output saturates, no unbounded hard clipping. */
 if(bite){
  int64_t x=sample;
  int64_t reduced=(x*x)>>15;
  sample=rupture_shape_clamp(x+((x*(32767-reduced)*bite)>>22));
 }
 /* Controlled initial-cycle transient, decays over first eight CXS cycles. */
 if(punch&&cycles<8u){
  uint32_t taper=8u-cycles;
  sample=rupture_shape_clamp((int64_t)sample+
           (((int64_t)sample*punch*taper)>>11));
 }
 return sample;
}
#endif
