#include "../src/rupture_data.h"
#include "../src/rupture_midi.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
static unsigned event_count;
static void cb(void*ctx,uint8_t s,uint8_t d1,uint8_t d2){(void)ctx;(void)s;(void)d1;(void)d2;event_count++;}
int main(void){
 RuptureLibrary *x=malloc(sizeof(*x));assert(x);rupture_library_init(x);assert(rupture_library_valid(x));assert(RUPTURE_PROJECTS==12&&RUPTURE_SEQ_SLOTS==8);
 for(int p=0;p<12;p++)for(int s=0;s<8;s++)assert(x->project[p].sequence[s].length==16);
 x->project[0].sequence[0].step[0].note=36;assert(!rupture_library_valid(x));rupture_library_seal(x);assert(rupture_library_valid(x));free(x);
 RuptureMidi m={0};uint8_t usb[]={0x09,0x90,36,127};rupture_midi_usb_event(&m,usb,cb,0);assert(m.notes==1);
 uint8_t trs[]={0x90,40,100,43,110,0x80,40,0,0xf8};for(unsigned i=0;i<sizeof(trs);i++)rupture_midi_byte(&m,trs[i],cb,0);
 assert(m.notes==3&&m.off==1&&m.clock==1&&event_count>=5);
 puts("PASS: 12x8 project schema, CRC integrity, USB-MIDI, TRS running status, MIDI clock");return 0;
}
