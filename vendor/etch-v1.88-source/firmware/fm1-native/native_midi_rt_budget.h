/* SPDX-License-Identifier: GPL-3.0-only
 * RUPTURE V1.85: deterministic USB-MIDI work budget per 128-frame block.
 * USB MIDI events are NEVER dropped by this budget. A remaining event stays
 * queued for the next DMA half; original overflow/panic handling remains.
 * 32 events per block covers extreme 12k-event/s bursts at 48kHz.
 */
#pragma once
#include <stdint.h>
#define RUPTURE_MIDI_EVENTS_PER_HALF 32u
static inline uint32_t ru_midi_batch_size(uint32_t pending){
 return pending<RUPTURE_MIDI_EVENTS_PER_HALF?pending:RUPTURE_MIDI_EVENTS_PER_HALF;
}
static inline uint8_t ru_midi_scope_skip(uint32_t remaining){
 return (uint8_t)(remaining>0u);
}
