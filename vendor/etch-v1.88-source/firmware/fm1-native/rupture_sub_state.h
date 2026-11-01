/* SPDX-License-Identifier: GPL-3.0-only
 * RUPTURE V1.11: tagged, backwards-compatible dual sub state.
 * Use unused ETCH synthesis fields in the EXISTING PJT32 33-byte DSP record.
 * No additional flash sectors, file records or layout changes.
 * Legacy ETCH modulation is never active for these tagged RUPTURE records:
 * matrix routes have source=0, living.depth=0, evolution.depth=0.
 * Tag spectral_wave bit6, independent from bit7 used for SOUND SHAPE.
 */
#pragma once
#include <stdint.h>
#include "native_project_codec.h"
#define RU_SUB_STATE_TAG 0x40u
static inline void ru_sub_state_defaults(uint8_t type[2],uint8_t oct[2],
 uint8_t detune[2],uint8_t drift[2]){
 type[0]=type[1]=0u;
 oct[0]=oct[1]=1u;
 detune[0]=50u;detune[1]=54u;
 drift[0]=drift[1]=0u;
}
static inline uint8_t ru_sub_state_valid(const struct etch_preset_dsp *v){
 if(!v || !(v->spectral_wave&RU_SUB_STATE_TAG))return 0u;
 if(v->living.depth || v->evolution.depth || v->evolution.mode || v->evolution.sync)return 0u;
 for(uint8_t i=0;i<4u;i++)if(v->matrix.route[i].source || v->matrix.route[i].target)return 0u;
 return (uint8_t)(v->matrix.route[0].amount<=4u &&
  v->matrix.route[1].amount<=3u && v->matrix.route[2].amount<=100u &&
  v->matrix.route[3].amount<=100u && v->living.rate<=4u &&
  v->living.color<=3u && v->living.vary<=100u && v->evolution.rate<=100u);
}
static inline void ru_sub_state_store(struct etch_preset_dsp *v,
 const uint8_t type[2],const uint8_t oct[2],
 const uint8_t detune[2],const uint8_t drift[2]){
 if(!v)return;
 for(uint8_t i=0;i<4u;i++){
  v->matrix.route[i].source=0u;v->matrix.route[i].target=0u;
 }
 v->matrix.route[0].amount=type[0]<=4u?type[0]:0u;
 v->matrix.route[1].amount=oct[0]<=3u?oct[0]:1u;
 v->matrix.route[2].amount=detune[0]<=100u?detune[0]:50u;
 v->matrix.route[3].amount=drift[0]<=100u?drift[0]:0u;
 v->living.depth=0u;
 v->living.rate=type[1]<=4u?type[1]:0u;
 v->living.color=oct[1]<=3u?oct[1]:1u;
 v->living.vary=detune[1]<=100u?detune[1]:54u;
 v->evolution.depth=0u;
 v->evolution.rate=drift[1]<=100u?drift[1]:0u;
 v->evolution.mode=0u;v->evolution.sync=0u;
 v->spectral_wave|=RU_SUB_STATE_TAG;
}
static inline uint8_t ru_sub_state_restore(const struct etch_preset_dsp *v,
 uint8_t type[2],uint8_t oct[2],uint8_t detune[2],uint8_t drift[2]){
 ru_sub_state_defaults(type,oct,detune,drift);
 if(!ru_sub_state_valid(v))return 0u;
 type[0]=v->matrix.route[0].amount;
 oct[0]=v->matrix.route[1].amount;
 detune[0]=v->matrix.route[2].amount;
 drift[0]=v->matrix.route[3].amount;
 type[1]=v->living.rate;
 oct[1]=v->living.color;
 detune[1]=v->living.vary;
 drift[1]=v->evolution.rate;
 return 1u;
}
