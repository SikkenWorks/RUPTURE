/* SPDX-License-Identifier: GPL-3.0-only
 * Scale guide + Keylock + deterministic mono arpeggiator.
 */
#ifndef RUPTURE_MUSIC_H
#define RUPTURE_MUSIC_H
#include <stdint.h>
#define RU_SCALE_CHROMATIC 0
#define RU_SCALE_MAJOR 1
#define RU_SCALE_MINOR 2
#define RU_SCALE_DORIAN 3
#define RU_SCALE_PENTATONIC 4
#define RU_SCALE_BLUES 5
#define RU_ARP_UP 0
#define RU_ARP_DOWN 1
#define RU_ARP_UPDOWN 2
#define RU_ARP_RANDOM 3
#define RU_ARP_ORDER 4
/* Return 1 if note is in selected scale. C=0, C#=1 etc. */
int rupture_in_scale(uint8_t note,uint8_t root,uint8_t scale);
/* Quantize to a musically legal semitone. Midpoint ties select lower. */
uint8_t rupture_keylock(uint8_t note,uint8_t root,uint8_t scale);
typedef struct {uint8_t mode,octaves,gate,rate,index,direction;uint32_t rng; } RuptureArp;
/* Input notes are current held pitches (last-note stack order); return pitch 0..127, or 255 if empty. */
uint8_t rupture_arp_next(RuptureArp *arp,const uint8_t *held,uint8_t count);
#endif
