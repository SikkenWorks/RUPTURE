/* SPDX-License-Identifier: GPL-3.0-only
 * RUPTURE V1.76: nearest-step recording quantizer, UI thread only.
 * Rewind from AUDIO playhead by first-contact timestamp from the 10kHz key
 * scanner. No extra audio callback computation or new MIDI/OTA routing.
 */
#ifndef RUPTURE_SEQ_QUANT_H
#define RUPTURE_SEQ_QUANT_H
#include <stdint.h>
/* MIDI/tempo step period is identical to perf_period() in audio_core.c. */
static inline uint32_t ru_quant_step_period(uint8_t bpm,uint8_t rate,uint8_t swing,uint16_t step){
 uint32_t tempo=bpm<30u?120u:bpm,div=rate>3u?8u:(1u<<rate);
 uint32_t base=(44100u*60u)/(tempo*div);
 if(!base)base=1u;
 if(swing){uint32_t offset=(base*(uint32_t)swing)/300u;
  base=(step&1u)?(base>offset?base-offset:1u):base+offset;}
 return base;
}
/* Convert 24 MHz TIMER4 ticks to samples. Ignore timestamps older than
 * 100 ms (e.g. a stale key edge during a suspended/blocked UI), to avoid
 * back-writing an unrelated step after a very long stall. */
static inline uint32_t ru_quant_age_samples(uint32_t elapsed_timer4_ticks){
 if(elapsed_timer4_ticks>2400000u)return 0u;
 uint32_t us=elapsed_timer4_ticks/24u;
 return (us*441u+5000u)/10000u;
}
/* current_step and remaining_frames are sampled while the audio IRQ is masked.
 * A key pressed in the later half of the current step belongs to NEXT step;
 * one pressed before the current boundary is snapped to current or PREVIOUS.
 * This is a true nearest-grid mapping, not UI-loop-time truncation. */
static inline uint16_t ru_quantize_rec_step(uint16_t current_step,uint16_t length,
 uint32_t remaining_frames,uint32_t elapsed_age_frames,uint8_t bpm,uint8_t rate,uint8_t swing){
 if(!length||current_step>=length)return current_step;
 uint32_t period=ru_quant_step_period(bpm,rate,swing,current_step);
 if(remaining_frames>=period)remaining_frames=period-1u;
 int32_t phase=(int32_t)(period-1u-remaining_frames)-(int32_t)elapsed_age_frames;
 if(phase>=0){
  if((uint32_t)phase>=period/2u)return (uint16_t)((current_step+1u)%length);
  return current_step;
 }
 uint16_t previous=(uint16_t)((current_step+length-1u)%length);
 uint32_t prev_period=ru_quant_step_period(bpm,rate,swing,previous);
 int32_t prev_phase=(int32_t)prev_period+phase;
 if(prev_phase<0)return previous;
 return (uint32_t)prev_phase>=prev_period/2u?current_step:previous;
}
#endif
