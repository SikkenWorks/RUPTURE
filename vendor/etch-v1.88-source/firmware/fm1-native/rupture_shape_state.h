/* SPDX-License-Identifier: GPL-3.0-only
 * RUPTURE V1.4: safe tagged per-preset SOUND SHAPE persistence.
 * RUPTURE uses CXS, not the legacy ETCH ORGANIC oscillator model.
 * Preserve the PJT32 33-byte DSP record and its existing A/B flash footprint.
 * Marker is the previously reserved bit 7 of spectral_wave. Unmarked (V1.3
 * or older) snapshots load neutral shaping, never garbage or legacy drift.
 * These four bytes may not be read as ETCH Organic settings in RUPTURE.
 */
#pragma once
#include <stdint.h>
#include "native_project_codec.h"
#define RU_SHAPE_TAG 0x80u
static inline void ru_shape_neutral(uint8_t *w,uint8_t *e,uint8_t *b,uint8_t *p){
 *w=0u;*e=50u;*b=0u;*p=0u;
}
static inline void ru_shape_store(struct etch_preset_dsp *v,
    uint8_t w,uint8_t e,uint8_t b,uint8_t p){
 if(!v)return;
 v->organic.drift=w>100u?100u:w;
 v->organic.rate=e>100u?100u:e;
 v->organic.phase=b>100u?100u:b;
 v->organic.variance=p>100u?100u:p;
 v->spectral_wave|=RU_SHAPE_TAG;
}
static inline uint8_t ru_shape_restore(const struct etch_preset_dsp *v,
    uint8_t *w,uint8_t *e,uint8_t *b,uint8_t *p){
 ru_shape_neutral(w,e,b,p);
 if(!v||!(v->spectral_wave&RU_SHAPE_TAG))return 0u;
 if(!etch_organic_valid(&v->organic))return 0u;
 *w=v->organic.drift;*e=v->organic.rate;
 *b=v->organic.phase;*p=v->organic.variance;
 return 1u;
}
