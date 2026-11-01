/* SPDX-License-Identifier: GPL-3.0-only
 * RUPTURE 24 dB/oct ladder: bounded, four-stage Q15 audio-rate kernel.
 *
 * V1.37: The former implementation performed five signed 64-bit
 * multiplies per output sample and voice. These are expensive in the FM1
 * ALNK DMA interrupt and can contribute to repeated late blocks followed
 * by the firmware's AUDIO STOP / Safe UI.
 *
 * This kernel is sample-for-sample identical to the former kernel for
 * legal Q15 audio input and bounded per-stage state. All multiplies fit
 * signed 32 bits. Do not turn off the deadline guard to hide underruns.
 */
#ifndef RUPTURE_LADDER_FAST_H
#define RUPTURE_LADDER_FAST_H
#include <stdint.h>

/* Prepared coefficients are shared across PCM frames and all voices that
 * use the same cutoff/resonance.  Normal patch parameters are usually stable
 * for many thousands of samples.  Only the main audio thread owns this cache.
 * No global state, heap allocations, or extra DSP POOL RAM are required. */
struct rupture_ladder_coeff {
 uint16_t cutoff, resonance;
 int32_t g, k_high, k_low;
};
static inline void rupture_ladder_prepare(struct rupture_ladder_coeff *c,
                                         uint16_t cutoff,uint16_t resonance){
 uint32_t f=cutoff>8192u?8192u:cutoff;
 uint32_t res=resonance>32767u?32767u:resonance;
 uint32_t k=(res*365u)/100u;
 c->cutoff=cutoff;c->resonance=resonance;
 c->g=(int32_t)(f*3u);c->k_high=(int32_t)(k>>15);
 c->k_low=(int32_t)(k&32767u);
}
static inline int32_t rupture_ladder_step_prepared(int32_t state[4],
                                                  const struct rupture_ladder_coeff *c,
                                                  int32_t input){
 /* The DSP signal is Q15. Limit corrupted upstream values before feedback.
  * (Normal Q15 inputs are not changed.) */
 if(input>32767)input=32767;
 if(input<-32768)input=-32768;
 int32_t z=state[3];
 /* Exact arithmetic decomposition: (z*k)>>15, even for negative z.
  * abs(z) <= 32768, k&32767 <=32767, product <= 1,073,709,056.
  * Whole part is at most 3, so the addition cannot overflow. */
 int32_t feedback=z*c->k_high+((z*c->k_low)>>15);
 int32_t x=input-feedback;
 if(x>32767)x=32767+(x-32767)/8;
 if(x< -32767)x=-32767+(x+32767)/8;
 if(x>49151)x=49151;
 if(x< -49151)x=-49151;
 for(uint32_t stage=0;stage<4u;stage++){
  z=state[stage];
  /* abs(x-z) <= 81919. 81919*24576 = 2,013,? < INT32_MAX.
   * The Q15 product therefore needs no 64-bit multiplication. */
  int32_t delta=((x-z)*c->g)>>15;
  z+=delta;
  if(z>32767)z=32767;
  if(z< -32767)z=-32767;
  state[stage]=z;
  x=z;
 }
 return state[3];
}
/* Compatibility entry point for the existing native/host callers. */
static inline int32_t rupture_ladder_fast_step(int32_t state[4],
                                             uint16_t cutoff,uint16_t resonance,
                                             int32_t input){
 struct rupture_ladder_coeff coeff;
 rupture_ladder_prepare(&coeff,cutoff,resonance);
 return rupture_ladder_step_prepared(state,&coeff,input);
}
#endif
