/* SPDX-License-Identifier: GPL-3.0-only
 * V2.0.38: read-only note-edge ALNK timing attribution.
 * Audio ownership, DSP samples, order and timer sources are unchanged.
 * Classification uses event-state deltas sampled once per 128-frame ISR;
 * never introduce a per-sample timestamp or a foreground lock. */
#ifndef RUPTURE_NOTE_DMA_METER_H
#define RUPTURE_NOTE_DMA_METER_H
#include <stdint.h>

enum {RU_NDMA_ON=1u,RU_NDMA_OFF=2u};
typedef struct {
 uint32_t on_events,off_events;
 uint32_t on_lates,off_lates;
 uint32_t on_wall_peak,off_wall_peak;
 uint32_t on_render_peak,off_render_peak;
} ru_note_dma_meter_t;

static inline uint8_t ru_note_dma_edge(uint32_t old_keys,uint32_t new_keys,
 uint32_t old_serial,uint32_t new_serial,uint8_t old_stack,uint8_t new_stack,
 uint8_t old_held,uint8_t new_held){
 uint8_t edges=0u;
 if((new_keys&~old_keys)||new_serial!=old_serial||new_stack>old_stack)
  edges|=RU_NDMA_ON;
 if((old_keys&~new_keys)||new_stack<old_stack||(old_held&&!new_held))
  edges|=RU_NDMA_OFF;
 return edges;
}

static inline void ru_note_dma_record(volatile ru_note_dma_meter_t *m,uint8_t edge,
 uint32_t render_ticks,uint32_t wall_ticks,uint8_t missed){
 if(!m||!edge)return;
 if(edge&RU_NDMA_ON){
  if(m->on_events!=UINT32_MAX)m->on_events++;
  if(missed&&m->on_lates!=UINT32_MAX)m->on_lates++;
  if(render_ticks>m->on_render_peak)m->on_render_peak=render_ticks;
  if(wall_ticks>m->on_wall_peak)m->on_wall_peak=wall_ticks;
 }
 if(edge&RU_NDMA_OFF){
  if(m->off_events!=UINT32_MAX)m->off_events++;
  if(missed&&m->off_lates!=UINT32_MAX)m->off_lates++;
  if(render_ticks>m->off_render_peak)m->off_render_peak=render_ticks;
  if(wall_ticks>m->off_wall_peak)m->off_wall_peak=wall_ticks;
 }
}
#endif
