/* SPDX-License-Identifier: GPL-3.0-only
 * V1.72: independent, beat-aligned short PCM click. This is mixed after the
 * RUPTURE mono synth/FX/idle gate. No voices, envelopes, delay, heap or flash.
 */
#ifndef RUPTURE_METRONOME_H
#define RUPTURE_METRONOME_H
#include <stdint.h>
/* Translate ALGORITHM detents to a session-only 0..100 click level.
 * Consume the encoder delta only during live recording; outside live REC
 * ALGORITHM must keep its existing sustain, sound-shaping and UI roles.
 * No synth voice or flash setting is involved. */
static inline uint8_t ru_metronome_record_algorithm(volatile uint8_t *level,
      int32_t *alg_delta,uint8_t recording,uint8_t playing){
 if(!recording||!playing||!*alg_delta)return 0u;
 int64_t next=(int64_t)*level+*alg_delta;
 if(next<0)next=0;else if(next>100)next=100;
 *level=(uint8_t)next;*alg_delta=0;
 return 1u;
}
#include "rupture_metronome_wave.h"
struct RuMetroClick {uint16_t pos;uint8_t sounding,accent;};
/* One quarter-note tick, regardless of sequencer subdivision. First tick
 * corresponds to the exact sample at which the sequencer triggers step 0. */
static inline void ru_metronome_step(struct RuMetroClick *m,
                                    uint16_t step,uint8_t seq_rate,uint8_t enabled){
 if(!enabled)return;
 const uint8_t div=(uint8_t)(seq_rate>=3u?8u:(1u<<seq_rate));
 if((step%div)!=0u)return;
 m->pos=0u;m->sounding=1u;
 m->accent=(uint8_t)((step%((uint16_t)div*4u))==0u);
}
/* Original 256-sample band-limited PCM click. This runs *after* the mono
 * synth and effects, independent of synth allocation and output idle gate.
 * One table read and one bounded 32-bit multiply per active output sample.
 * No extra work at all beyond the flag check when REC is off. */
static inline int16_t ru_metronome_sample(struct RuMetroClick *m,
                                         uint8_t enabled,uint8_t volume){
 if(!enabled || !volume){m->sounding=0u;m->pos=0u;return 0;}
 if(!m->sounding)return 0;
 const uint16_t pos=m->pos++;
 if(m->pos>=256u)m->sounding=0u;
 const int32_t base=ru_metro_pcm[pos];
 const int32_t level=(int32_t)volume*(m->accent?3:2);
 return (int16_t)((base*level)>>8);
}

#endif
