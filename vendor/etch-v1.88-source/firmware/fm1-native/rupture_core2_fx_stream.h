/* SPDX-License-Identifier: GPL-3.0-only
 * RUPTURE V2.0.58: one mailbox, two immutable 64-frame producer windows.
 * CPU0 publishes only completed dry samples; CPU1 alone owns ordered FX.
 * Pure control-plane policy: no CXS or effect-sample equations are changed.
 */
#ifndef RUPTURE_CORE2_FX_STREAM_H
#define RUPTURE_CORE2_FX_STREAM_H
#include <stdint.h>
enum {RU_C1_FX_STREAM_FRAMES=128u,RU_C1_FX_STREAM_CHUNK=64u};
static inline __attribute__((always_inline)) uint8_t ru_c1_fx_stream_start(uint32_t begin,uint32_t end,
 uint32_t ready,uint32_t progress){
 return (uint8_t)(begin==0u&&end==RU_C1_FX_STREAM_FRAMES&&
   progress==0u&&ready>=RU_C1_FX_STREAM_CHUNK&&
   ready<=RU_C1_FX_STREAM_FRAMES&&
   (ready%RU_C1_FX_STREAM_CHUNK)==0u);
}
/* Fences belong to callers; the watermark itself must never point beyond
 * the two complete, non-overlapping settings/sample snapshots. */
static inline __attribute__((always_inline)) uint8_t ru_c1_fx_stream_watermark(uint32_t progress,
 uint32_t ready){
 return (uint8_t)(progress<=RU_C1_FX_STREAM_FRAMES&&
  (progress%RU_C1_FX_STREAM_CHUNK)==0u&&
  ready>=progress&&ready<=RU_C1_FX_STREAM_FRAMES&&
  (ready%RU_C1_FX_STREAM_CHUNK)==0u);
}
static inline __attribute__((always_inline)) uint8_t ru_c1_fx_stream_publish(uint32_t previous,
 uint32_t ready){
 return (uint8_t)(previous>=RU_C1_FX_STREAM_CHUNK&&
  previous<RU_C1_FX_STREAM_FRAMES&&
  ready==previous+RU_C1_FX_STREAM_CHUNK);
}
#endif
