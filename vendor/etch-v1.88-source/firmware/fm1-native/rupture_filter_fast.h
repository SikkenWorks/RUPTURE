/* SPDX-License-Identifier: GPL-3.0-only
 * RUPTURE V1.38: integer-division-free Chamberlin SVF core.
 * Matches the original bounded Q15 filter exactly for all supported
 * coefficient and state values; it does not change the filter response.
 * The arithmetic uses 32-bit multiplication, no 64-bit division and
 * no variable integer division inside the audio interrupt.
 */
#ifndef RUPTURE_FILTER_FAST_H
#define RUPTURE_FILTER_FAST_H
#include <stdint.h>

static inline int32_t rupture_svf_bound(int32_t x){
 return x < -262144 ? -262144 : x > 262144 ? 262144 : x;
}
/* Q15 multiplication with C's signed truncation-toward-zero semantics.
 * |state| <= 262144, 0 <= coefficient <= 65536.
 * remainder * coefficient <= 32767*65536 = 2147418112 < INT32_MAX.
 * The quotient's product <= 8*65536. Avoid signed right-shift rules
 * by shifting the non-negative magnitude and restoring the sign. */
static inline int32_t rupture_svf_mul(int32_t state,uint32_t coefficient){
 uint32_t magnitude=(uint32_t)(state<0?-state:state);
 uint32_t high=(magnitude>>15u)*coefficient;
 uint32_t low=((magnitude&32767u)*coefficient)>>15u;
 int32_t out=(int32_t)(high+low);
 return state<0?-out:out;
}
/* Original two-iteration Chamberlin state machine. All intermediate
 * values bounded, including when coefficients are invalid or extreme.
 * No output saturation is performed here, leaving callers in control
 * of their existing +/-32768 audio boundary. */
struct rupture_svf_coeff {uint16_t cutoff,resonance;uint32_t f,damping;};
static inline void rupture_svf_prepare(struct rupture_svf_coeff *c,
                                      uint16_t cutoff,uint16_t resonance){
 uint32_t res=resonance>32767u?32767u:resonance;
 c->cutoff=cutoff;c->resonance=resonance;
 c->f=cutoff>8192u?8192u:cutoff;
 c->damping=65536u-(res*3u)/2u;
}
static inline int32_t rupture_svf_step_prepared(int32_t *low,int32_t *band,
                           const struct rupture_svf_coeff *c,int32_t input,uint8_t type){
 int32_t high=0;
 for(uint32_t k=0;k<2u;k++){
  *low=rupture_svf_bound(*low+rupture_svf_mul(*band,c->f));
  high=rupture_svf_bound(input-*low-rupture_svf_mul(*band,c->damping));
  *band=rupture_svf_bound(*band+rupture_svf_mul(high,c->f));
 }
 return type==2u?*band:type==3u?high:*low;
}
static inline int32_t rupture_svf_step(int32_t *low,int32_t *band,
                           uint16_t cutoff,uint16_t resonance,
                           int32_t input,uint8_t type){
 struct rupture_svf_coeff coeff;
 rupture_svf_prepare(&coeff,cutoff,resonance);
 return rupture_svf_step_prepared(low,band,&coeff,input,type);
}
#endif
