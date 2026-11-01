/* SPDX-License-Identifier: GPL-3.0-only
 * RUPTURE CXS monophonic six-note memory.
 * Bounded by six, no allocation, independent of waveform rendering.
 */
#ifndef RUPTURE_NOTE_MEMORY_H
#define RUPTURE_NOTE_MEMORY_H
#include <stdint.h>
#define RUPTURE_NOTE_LIMIT 6u
#define RU_PRIORITY_LAST 0u
#define RU_PRIORITY_HIGH 1u
#define RU_PRIORITY_LOW 2u
#define RU_SOURCE_USB 0u
#define RU_SOURCE_TRS 1u
#define RU_SOURCE_PANEL 2u
#define RU_SOURCE_SEQ 3u
#define RU_SOURCE_COUNT 4u

typedef struct {uint16_t identity;uint8_t pitch,velocity,source,channel,down;} RuNote;
typedef struct {
 RuNote notes[RUPTURE_NOTE_LIMIT];
 uint8_t count,priority,legato;
 uint16_t sustain[RU_SOURCE_COUNT]; /* bitset channels 0..15 */
 uint32_t glide_samples;
} RuNoteMemory;
typedef struct {
 uint8_t active,pitch,velocity,source,channel,retrigger,glide;
} RuNoteDecision;
void ru_note_init(RuNoteMemory *m);
/* Note-on and -off are paired by original identity, not quantized pitch.
 * Repeated note-ons pair FIFO; a lost oldest item can no longer be recalled.
 * A velocity-zero on must be delivered as note-off by the caller. */
RuNoteDecision ru_note_on(RuNoteMemory *m,uint8_t src,uint8_t channel,uint8_t note,uint8_t tuned,uint8_t velocity);
RuNoteDecision ru_note_off(RuNoteMemory *m,uint8_t src,uint8_t channel,uint8_t note);
RuNoteDecision ru_note_sustain(RuNoteMemory *m,uint8_t src,uint8_t channel,uint8_t enabled);
RuNoteDecision ru_note_panic(RuNoteMemory *m,uint8_t src,uint8_t channel,uint8_t all_channels);
RuNoteDecision ru_note_current(const RuNoteMemory *m);
#endif
