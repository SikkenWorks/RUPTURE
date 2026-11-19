#ifndef RUPTURE_NOTE_WARMUP_GATE_H
#define RUPTURE_NOTE_WARMUP_GATE_H
#include <stdint.h>

/* The FM1 analog output is intentionally muted during LCD/codec settle.
 * Do not START the first local notes invisibly while digital audio is still
 * silent: a held key must trigger on the first actually audible DMA block.
 * This does not alter sample/voice math after the gate is released. */
static inline uint32_t ru_note_keys_while_output_gated(uint32_t raw_keys,
                                                        uint8_t output_enabled){
 return output_enabled ? raw_keys : 0u;
}

#endif
