/* SPDX-License-Identifier: GPL-3.0-only
 * RUPTURE V1.7 - selected-step velocity and guarded idle autosave.
 */
#ifndef RUPTURE_SEQ_POLICY_H
#define RUPTURE_SEQ_POLICY_H
#include <stdint.h>
#include "native_performance.h"
static inline uint8_t rupture_step_velocity(struct etch_native_pattern *p, uint16_t step, uint8_t velocity) {
 if(!p || step >= ETCH_SEQ_STEPS || velocity < 1u || velocity > 127u)return 0u;
 struct etch_seq_step *s=&p->steps[step];
 if(s->count > ETCH_SEQ_NOTES)return 0u;
 for(uint8_t i=0;i<s->count;i++)s->notes[i].velocity=velocity;
 return s->count;
}
/* At least 20 seconds after last edit, at least 60 seconds between flash
 * commits; never write during transport, held notes, faults or updates. */
static inline uint8_t rupture_autosave_due(uint32_t now,uint32_t dirty_at,uint32_t last_write,
 uint8_t dirty,uint8_t already_saved,uint8_t running,uint8_t notes_held,uint8_t blocked){
 return (uint8_t)(dirty && already_saved && !running && !notes_held && !blocked
     && (uint32_t)(now-dirty_at)>=20000u
     && (last_write==0u || (uint32_t)(now-last_write)>=60000u));
}
#endif
