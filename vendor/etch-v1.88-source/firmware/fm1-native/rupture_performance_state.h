/* SPDX-License-Identifier: GPL-3.0-only
 * RUPTURE V1.12: tagged NOTE MEMORY and volume persistence for CXS presets.
 * Preserve PJT32 33-byte ABI and existing A/B flash sector boundaries.
 * RUPTURE does not use ETCH's legacy spectral/blend controls in CXS mode.
 * spectral_part=31, spectral_tilt=97 and blend_on=1 are a 3-field marker.
 * All numeric fields are validated by the original PJT32 decoder.
 */
#pragma once
#include <stdint.h>
#include "native_project_codec.h"
#include "rupture_patch_state.h"
#define RU_PERF_PART_TAG 31u
#define RU_PERF_TILT_TAG 97u
static inline void ru_perf_defaults(uint8_t *master,uint8_t *priority,
 uint8_t *legato,uint16_t *glide_ms,uint8_t *bend){
 *master=100u;*priority=0u;*legato=1u;*glide_ms=0u;*bend=2u;
}
static inline void ru_perf_store(struct etch_preset_dsp *v,
 uint8_t master,uint8_t priority,uint8_t legato,uint16_t glide_ms,uint8_t bend){
 if(!v)return;
 if(master>200u)master=200u;
 if(priority>2u)priority=0u;
 if(legato>1u)legato=1u;
 if(glide_ms>400u)glide_ms=400u;
 if(bend>12u)bend=12u;
 uint32_t packed=(uint32_t)master;
 packed+=201u*( (uint32_t)priority +3u*( (uint32_t)legato +2u*( (uint32_t)(glide_ms/5u)+81u*(uint32_t)bend)));
 v->blend_a=(uint8_t)(packed%32u);packed/=32u;
 v->blend_b=(uint8_t)(packed%32u);packed/=32u;
 v->blend_pct=(uint8_t)(packed%101u);packed/=101u;
 v->spectral_gain=(uint8_t)packed;
 v->spectral_part=RU_PERF_PART_TAG;
 v->spectral_tilt=RU_PERF_TILT_TAG;
 v->blend_on=1u;
}
static inline uint8_t ru_perf_restore(const struct etch_preset_dsp *v,
 const struct etch_native_patch *patch,uint8_t *master,uint8_t *priority,
 uint8_t *legato,uint16_t *glide_ms,uint8_t *bend){
 ru_perf_defaults(master,priority,legato,glide_ms,bend);
 if(!v || !patch || !ru_patch_is_cxs(patch) ||
   v->spectral_part!=RU_PERF_PART_TAG ||
   v->spectral_tilt!=RU_PERF_TILT_TAG || v->blend_on!=1u ||
   v->blend_a>=32u || v->blend_b>=32u ||
   v->blend_pct>100u || v->spectral_gain>100u)return 0u;
 uint32_t packed=(uint32_t)v->blend_a +32u*((uint32_t)v->blend_b +32u*((uint32_t)v->blend_pct+101u*(uint32_t)v->spectral_gain));
 uint8_t m=(uint8_t)(packed%201u);packed/=201u;
 uint8_t pr=(uint8_t)(packed%3u);packed/=3u;
 uint8_t le=(uint8_t)(packed%2u);packed/=2u;
 uint16_t gl=(uint16_t)((packed%81u)*5u);packed/=81u;
 if(packed>12u)return 0u;
 *master=m;*priority=pr;*legato=le;*glide_ms=gl;*bend=(uint8_t)packed;
 return 1u;
}
