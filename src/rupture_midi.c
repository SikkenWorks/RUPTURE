/* SPDX-License-Identifier: GPL-3.0-only */
#include "rupture_midi.h"
static uint8_t want(uint8_t s){uint8_t n=s&0xf0;return (n==0xc0||n==0xd0)?1:(n>=0x80&&n<=0xe0)?2:0;}
void rupture_midi_byte(RuptureMidi *m,uint8_t b,RuptureMidiEvent deliver,void *ctx){
 if(b>=0xf8){if(b==0xf8)m->clock++;if(deliver)deliver(ctx,b,0,0);return;}
 if(b&0x80){m->status=(b>=0xf0)?0:b;m->needed=want(m->status);m->used=0;return;}
 if(!m->needed||!m->status)return;
 m->bytes[m->used++]=b;
 if(m->used>=m->needed){uint8_t s=m->status,d1=m->bytes[0],d2=m->needed>1?m->bytes[1]:0;
 if((s&0xf0)==0x90&&d2)m->notes++;
 if((s&0xf0)==0x80||((s&0xf0)==0x90&&d2==0))m->off++;
 if(deliver)deliver(ctx,s,d1,d2);
 m->used=0;}
}
void rupture_midi_usb_event(RuptureMidi *m,const uint8_t packet[4],RuptureMidiEvent deliver,void *ctx){
 const uint8_t cin=packet[0]&0xf;uint8_t count=(cin==5||cin==0xf)?1:(cin==2||cin==6||cin==0xc||cin==0xd)?2:(cin==3||cin==4||cin==7||cin==8||cin==9||cin==0xa||cin==0xb||cin==0xe)?3:0;
 for(uint8_t i=0;i<count;i++)rupture_midi_byte(m,packet[1+i],deliver,ctx);
}
