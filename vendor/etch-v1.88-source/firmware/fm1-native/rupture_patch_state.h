/* SPDX-License-Identifier: GPL-3.0-only
 * RUPTURE CXS patch-state compatibility in existing ETCH 48-byte records.
 * Mark RUPTURE-owned records via bit 7 of the existing `valid` byte, so
 * legacy records cannot be accidentally decoded as CXS knobs.
 * Uses two otherwise unused morph/depth words; no on-flash format expansion.
 * These words no longer carry ETCH morph/depth in RUPTURE-marked presets.
 */
#pragma once
#include <stdint.h>
#include "native_patch.h"
static uint8_t ru_patch_is_cxs(const struct etch_native_patch *p){
 if(!p || !(p->valid&0x80u))return 0u;
 return (uint8_t)((p->morph>>7)<=100u && (p->morph&127u)<=100u &&
                 (p->depth>>7)<=100u && (p->depth&127u)<=100u);
}
static void ru_patch_cxs_save(struct etch_native_patch *p,uint8_t pressure,uint8_t balance,uint8_t memory,uint8_t density){
 p->valid=0x81u;
 p->morph=(uint16_t)(((uint16_t)(pressure>100u?100u:pressure)<<7) | (balance>100u?100u:balance));
 p->depth=(uint16_t)(((uint16_t)(memory>100u?100u:memory)<<7) | (density>100u?100u:density));
}
static uint8_t ru_patch_cxs_load(const struct etch_native_patch *p,uint8_t *pressure,uint8_t *balance,uint8_t *memory,uint8_t *density){
 if(!ru_patch_is_cxs(p))return 0;
 *pressure=(uint8_t)(p->morph>>7);*balance=(uint8_t)(p->morph&127u);
 *memory=(uint8_t)(p->depth>>7);*density=(uint8_t)(p->depth&127u);
 return 1u;
}
