/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef RUPTURE_MIDI_H
#define RUPTURE_MIDI_H
#include <stdint.h>
typedef struct {uint8_t status,needed,used,bytes[2];uint32_t notes,off,clock;} RuptureMidi;
typedef void (*RuptureMidiEvent)(void*,uint8_t status,uint8_t data1,uint8_t data2);
void rupture_midi_byte(RuptureMidi *m,uint8_t byte,RuptureMidiEvent deliver,void *ctx);
/* USB-MIDI: one 4-byte USB event packet, cable + CIN in byte 0. */
void rupture_midi_usb_event(RuptureMidi *m,const uint8_t packet[4],RuptureMidiEvent deliver,void *ctx);
#endif
