/* SPDX-License-Identifier: GPL-3.0-only
 * Fixed-point pi32v2-compatible CXS oscillator kernel, isolated module.
 * Experimental DSP: no device I/O and not yet integrated into V1.88.
 */
#ifndef RUPTURE_NATIVE_CXS_Q15_H
#define RUPTURE_NATIVE_CXS_Q15_H
#include <stdint.h>
typedef struct {
 uint32_t phase,increment,seed,cycles;
 uint16_t durations[8],boundaries[8];
 int16_t flux[8],levels[8];
 int32_t dc,smooth;
 uint8_t pressure,balance,memory,density,edge;
} RuptureCXSQ15;
void rupture_q15_init(RuptureCXSQ15 *s,uint32_t note_increment);
void rupture_q15_params(RuptureCXSQ15 *s,uint8_t pressure,uint8_t balance,uint8_t memory,uint8_t density);
int16_t rupture_q15_sample(RuptureCXSQ15 *s);
int rupture_q15_valid(const RuptureCXSQ15 *s);
#endif
