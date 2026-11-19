/* SPDX-License-Identifier: GPL-3.0-only
 * V2.0.39: defer OPTIONAL oscilloscope preparation when heavy FX and note
 * edges compete for ALNK0 time. The PCM, FX, MIDI and USB paths are not
 * conditional on this guard. One call per 128-frame DMA half only. */
#ifndef RUPTURE_NOTE_SCOPE_GUARD_H
#define RUPTURE_NOTE_SCOPE_GUARD_H
#include <stdint.h>

enum { RU_SCOPE_FX_QUIET_HALVES=24u };
typedef struct { uint8_t quiet_halves; } ru_note_scope_guard_t;

static inline uint8_t ru_note_scope_capture_allow(
 ru_note_scope_guard_t *state,uint8_t ordinary_scope_allow,
 uint8_t note_edge,uint8_t active_fx,uint32_t previous_slack,
 uint32_t half_budget){
 if(!state)return ordinary_scope_allow;
 /* 24 blocks is ~64ms at 48k and ~70ms at 44.1k. Do not defer
  * audio, only expensive waveform capture and averaging. */
 if(note_edge&&active_fx>=3u)state->quiet_halves=RU_SCOPE_FX_QUIET_HALVES;
 uint8_t heavy=(uint8_t)(active_fx>=3u);
 uint8_t quiet=(uint8_t)(heavy&&state->quiet_halves);
 /* Near-exhausted deadline is critical even with only 0-2 FX. Always
  * prioritize real audio over the optional scope in that condition. */
 uint8_t low_slack=(uint8_t)(half_budget&&
                            previous_slack<=half_budget/5u);
 if(state->quiet_halves)--state->quiet_halves;
 return (uint8_t)(ordinary_scope_allow&&!quiet&&!low_slack);
}
#endif
