/* SPDX-License-Identifier: GPL-3.0-only
 * RUPTURE Channel EQ 2.0: four RBJ biquads / signed fixed-point.
 * Table generator: tools/generate_eq2_tables.py. 44.1kHz native DAC path.
 * Coefficient calculations are CONTROL-THREAD ONLY, no trig/divide in IRQ.
 */
#pragma once
#include <stdint.h>
#define RU_EQ2_SHIFT 26
#define RU_EQ2_ONE (1L<<RU_EQ2_SHIFT)
struct rupture_eq2_params {uint8_t enabled;uint8_t freq[4];uint8_t q[2];};
struct rupture_eq2_bq {int32_t b0,b1,b2,a1,a2;};
struct rupture_eq2_state {struct rupture_eq2_bq target[2][4],current[4];int32_t x1[4],x2[4],y1[4],y2[4];uint16_t headroom_q15[2],current_headroom;uint8_t index;uint8_t initialized;uint8_t slew_phase;uint8_t slew_active;uint8_t unity_hint;uint8_t active_mask;};
static inline void rupture_eq2_defaults(struct rupture_eq2_params *p,uint8_t enabled){
 p->enabled=enabled;for(uint8_t i=0;i<4;i++)p->freq[i]=50;
 p->q[0]=9;p->q[1]=9;
}
static inline uint8_t rupture_eq2_valid(const struct rupture_eq2_params *p){
 if(p->enabled>1)return 0;
 for(uint8_t i=0;i<4;i++)if(p->freq[i]>100)return 0;
 return p->q[0]<=100&&p->q[1]<=100;
}
void rupture_eq2_compute(const struct rupture_eq2_params *p,const uint16_t gains[4],struct rupture_eq2_bq out[4]);
void rupture_eq2_publish(struct rupture_eq2_state *s,const struct rupture_eq2_bq coeff[4],const uint16_t gains[4]);
int32_t rupture_eq2_process(struct rupture_eq2_state *s,int32_t x);
/* A four-band 0 dB EQ must be an exact zero-CPU DSP bypass once its
 * existing coefficient/headroom ramps have settled. */
uint8_t rupture_eq2_is_unity(const struct rupture_eq2_state *s);
uint16_t rupture_eq2_frequency(uint8_t band,uint8_t index);
uint16_t rupture_eq2_gain_index(uint16_t legacy_q15);
void rupture_eq2_response_curve(const struct rupture_eq2_bq c[4],uint8_t out[54]);
