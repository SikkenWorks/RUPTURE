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
 /* 220 Hz approx at 44.1kHz. One existing one-pole state is reused.
  * Both controls have musically distinct, SUSTAINED frequency response:
  * WEIGHT: gentle bass shelf plus controlled high-band attenuation.
  * PUNCH: high-band transient/edge emphasis throughout the held note,
  *        retaining the familiar first-eight-cycle attack accent.
  * At WEIGHT=PUNCH=0 the original PCM is bit-identical; no new state or
  * buffer is needed, and all products fit within signed 32 bits. */
 *body_low+=(sample-*body_low)/32;
 if(weight){
  int32_t low=*body_low;
  int32_t high=sample-low;
  int32_t weighted=(low*(256+(int32_t)weight*2)+
                    high*(256-(int32_t)weight))>>8;
  sample=rupture_shape_clamp(weighted);
 }
 /* Original odd-order soft saturation / bite preserved. */
 if(bite){
  int64_t x=sample;
  int64_t reduced=(x*x)>>15;
  sample=rupture_shape_clamp(x+((x*(32767-reduced)*bite)>>22));
 }
 if(punch){
  /* Persistent emphasis: never stop PUNCH after oscillator cycle eight.
   * 3/512 scale gives a strong but bounded colour at high knob settings. */
  int32_t low=*body_low;
  int32_t high=sample-low;
  /* Independent high-band boost / low-band trim, not a volume knob. */
  int32_t shaped=((low*(256-(int32_t)punch*2)) +
                  (high*(256+(int32_t)punch)))>>8;
  sample=rupture_shape_clamp(shaped);
  if(cycles<8u){
   uint32_t taper=8u-cycles;
   sample=rupture_shape_clamp((int64_t)sample+
          (((int64_t)sample*punch*taper)>>12));
  }
 }
 return sample;
}
#endif
