/* SPDX-License-Identifier: GPL-3.0-only
 * RUPTURE V1.45: main oscillator selector in unused DSP bit 4.
 * No new flash records. All old projects restore CXS.
 */
#ifndef RU_MAIN_STATE_H
#define RU_MAIN_STATE_H
#include <stdint.h>
#include "native_project_codec.h"
#define RU_MAIN_SAW_TAG 0x10u
static inline void ru_main_state_store(struct etch_preset_dsp *v,uint8_t wave){
 if(wave)v->spectral_wave|=RU_MAIN_SAW_TAG;
 else v->spectral_wave&=(uint8_t)~RU_MAIN_SAW_TAG;
}
static inline uint8_t ru_main_state_restore(const struct etch_preset_dsp *v){
 return (uint8_t)!!(v->spectral_wave&RU_MAIN_SAW_TAG);
}
#endif
