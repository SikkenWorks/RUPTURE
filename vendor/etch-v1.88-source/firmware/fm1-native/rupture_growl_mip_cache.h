/* RUPTURE V2.0.25: exact cache for GROWL pitch-dependent band selection.
 * Keep this separate from the automatically generated wave-table header.
 * SPDX-License-Identifier: GPL-3.0-only */
#ifndef RUPTURE_GROWL_MIP_CACHE_H
#define RUPTURE_GROWL_MIP_CACHE_H
#include <stdint.h>
#include "rupture_growl_waves.h"

/* Preserve all original mip lookup/interpolation operations. Cache only the
 * band decision across samples with EXACTLY the same phase increment.
 * Glide, bend, LFO and sequencer pitch changes recompute immediately;
 * phase itself is never cached or approximated. */
typedef struct {
 uint32_t increment;
 uint16_t mix;
 uint8_t level,blend,valid;
} RuGrowlMipCache;
static inline int32_t ru_growl_read_cached(uint32_t phase,uint32_t increment,
                                         RuGrowlMipCache *cache){
 if(!cache)return ru_growl_read(phase,increment);
 if(!cache->valid||cache->increment!=increment){
  uint8_t level=ru_wave_band(increment);
  uint16_t mix=0u;
  uint8_t blended=0u;
  if(level){
   uint32_t threshold=0x7fffffffu>>level;
   uint32_t fade_range=threshold>>3;
   if(increment>threshold-fade_range && increment<=threshold){
    uint32_t old_mix=(threshold-increment)>>(20u-level);
    if(old_mix>256u)old_mix=256u;
    mix=(uint16_t)old_mix;
    blended=1u;
   }
  }
  cache->increment=increment;
  cache->level=level;
  cache->mix=mix;
  cache->blend=blended;
  cache->valid=1u;
 }
 int32_t now=ru_growl_level_read(cache->level,phase);
 if(cache->blend){
  int32_t old=ru_growl_level_read((uint8_t)(cache->level-1u),phase);
  now=((now*(int32_t)cache->mix)+
       (old*(int32_t)(256u-cache->mix)))>>8;
 }
 return now;
}
#endif
