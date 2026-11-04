/* SPDX-License-Identifier: GPL-3.0-only
 * RUPTURE V1.75. Exact 2-bar record count-in at 44.1 kHz.
 * Shared click voice only: never allocates synth voices, effects or flash.
 * Main-thread begin/cancel must be called with audio IRQ disabled.
 */
#ifndef RUPTURE_COUNTIN_H
#define RUPTURE_COUNTIN_H
#include <stdint.h>
#include "rupture_metronome.h"
struct RuCountIn {
 uint32_t to_next;
 uint8_t active,beats,ready,force_seq,first_step_guard;
};
static inline void ru_countin_cancel(volatile struct RuCountIn *c){
 c->active=0u;c->ready=0u;c->force_seq=0u;c->beats=0u;c->to_next=0u;c->first_step_guard=0u;
}
static inline void ru_countin_begin(volatile struct RuCountIn *c){
 ru_countin_cancel(c);c->active=1u;
}
/* Called exactly once per rendered sample. The first click occurs at the
 * count-in start sample. Eight quarter-note clicks, with strong beat 1 of
 * each bar, then transport begins on the NINTH beat boundary (step 1).
 * The return value is 1 only on that precise launch sample. Tempo changes
 * are applied on the next beat, so detents don't cause a discontinuity.
 */
static inline uint8_t ru_countin_sample(volatile struct RuCountIn *c,
 struct RuMetroClick *click,uint8_t bpm){
 if(!c->active)return 0u;
 if(c->to_next){--c->to_next;return 0u;}
 if(c->beats==8u){
  c->active=0u;c->ready=1u;c->force_seq=1u;c->first_step_guard=1u;return 1u;
 }
 click->pos=0u;click->sounding=1u;click->accent=(uint8_t)((c->beats%4u)==0u);
 ++c->beats;
 uint32_t tempo=bpm<30u?120u:bpm;
 uint32_t interval=(44100u*60u)/tempo;
 c->to_next=(interval?interval:1u)-1u;
 return 0u;
}
#endif
