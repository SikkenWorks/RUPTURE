/* SPDX-License-Identifier: GPL-3.0-only
 * LCD-side sequencer playhead tracking.  Audio/LED timing stays authoritative.
 */
#ifndef RUPTURE_SEQ_REDRAW_H
#define RUPTURE_SEQ_REDRAW_H
#include <stdint.h>
static inline uint8_t ru_seq_playhead_needs_paint(uint8_t sequencer_page,
 uint8_t running,uint8_t screensaver,uint16_t audible_step,uint16_t painted_step){
 return (uint8_t)(sequencer_page && running && !screensaver &&
                   audible_step!=painted_step);
}
#endif
