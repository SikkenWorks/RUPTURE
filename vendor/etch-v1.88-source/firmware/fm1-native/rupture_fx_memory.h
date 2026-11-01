/* SPDX-License-Identifier: GPL-3.0-only
 * RUPTURE PJT33: independent project-scoped post-mix FX parameters and bypass.
 * The fixed 21-byte extension is appended ONLY to PJT33 projects.
 */
#pragma once
#include <stdint.h>
#define RU_FXM_BYTES 21u
#define RU_FXM_EXTRA_COUNT 4u
struct rupture_fx_memory {
 uint8_t knobs[4][4]; /* [0][0..1] SUB BLEND when enabled bit4 tagged; TILT, FUZZ, SANDPAPER */
 uint8_t enabled;     /* bits 1..3 FX, bit4 SUB BLEND, bit5 EQ AIR +/- marker */
 uint32_t bypass;     /* native ETCH FX mask, 22 meaningful bits */
};
static inline void ru_fxm_defaults(struct rupture_fx_memory *x){
 static const uint8_t defaults[4][4]={{75,75,60,65},{50,50,65,50},{55,75,65,65},{60,40,45,80}};
 for(uint8_t e=0;e<4u;e++)for(uint8_t k=0;k<4u;k++)x->knobs[e][k]=defaults[e][k];
 x->enabled=0u;x->bypass=0u;
}
static inline uint8_t ru_fxm_valid(const struct rupture_fx_memory *x){
 if(x->enabled&0xc0u)return 0u;
 if(x->bypass&~((1u<<22u)-1u))return 0u;
 for(uint8_t e=0;e<4u;e++)for(uint8_t k=0;k<4u;k++)if(x->knobs[e][k]>100u)return 0u;
 return 1u;
}
