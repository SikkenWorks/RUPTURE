/* SPDX-License-Identifier: GPL-3.0-only
 * RUPTURE V1.76: last-beat / step-one quantization handshake.
 * This is UI-only logic, with no allocation, DSP or OTA dependency.
 */
#ifndef RUPTURE_COUNTIN_BRIDGE_H
#define RUPTURE_COUNTIN_BRIDGE_H
#include <stdint.h>
#include "rupture_seq_quant.h"
/* A physical press at most 50ms BEFORE the next transport downbeat is
 * considered on step 1; note edges earlier in the count-in remain ignored.
 * `remaining_frames` and `age_samples` are both audio-sample counts.
 * Pure function for host tests; the audio thread never calls this. */
static inline uint8_t ru_countin_qualifies_prepress(uint8_t beats,
 uint32_t remaining_frames,uint32_t age_samples){
 return (uint8_t)(beats==8u && age_samples<=2205u &&
                   remaining_frames<=2205u-age_samples);
}
/* On the initial post-countdown step, do not wrap a late-processed
 * timestamp back into step N (the last step of the previous loop).
 * Normal nearest-grid snapping resumes on the following sequencer step. */
static inline uint16_t ru_countin_clamp_first_step(uint16_t quant_step,
 uint16_t current_step,uint16_t length,uint8_t first_step_guard){
 if(first_step_guard && current_step==0u && length && quant_step==length-1u)
  return 0u;
 return quant_step;
}
#endif
