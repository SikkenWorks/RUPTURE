#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_trs_input.h"
static uint32_t send(RuTrsParser *p,uint8_t b){uint32_t result=0;return ru_trs_feed(p,b,&result)?result:0;}
int main(void){
 RuTrsParser p;ru_trs_reset(&p);
 assert(send(&p,0x90)==0);assert(send(&p,36)==0);
 assert(send(&p,100)==(0x09u|(0x90u<<8)|(36u<<16)|(100u<<24)));
 assert(send(&p,38)==0);assert(send(&p,110)==(0x09u|(0x90u<<8)|(38u<<16)|(110u<<24)));
 assert(send(&p,40)==0);assert(send(&p,0xF8)==(0x0Fu|(0xF8u<<8)));
 assert(send(&p,120)==(0x09u|(0x90u<<8)|(40u<<16)|(120u<<24)));
 assert(send(&p,0x80)==0);assert(send(&p,40)==0);
 assert(send(&p,0)==(0x08u|(0x80u<<8)|(40u<<16)));
 assert(send(&p,0xC3)==0);assert(send(&p,7)==(0x0Cu|(0xC3u<<8)|(7u<<16)));
 assert(send(&p,0xF2)==0);assert(send(&p,0x7f)==0);assert(send(&p,0x7f)==0);
 assert(send(&p,7)==0); /* system common cleared running status */
 assert(send(&p,0xF0)==0);for(int i=0;i<30;i++)assert(send(&p,(uint8_t)(i&127))==0);
 assert(send(&p,0xF7)==0);assert(send(&p,12)==0);
 assert(send(&p,0xB0)==0);assert(send(&p,64)==0);
 assert(send(&p,127)==(0x0Bu|(0xB0u<<8)|(64u<<16)|(127u<<24)));
 assert(send(&p,0xE0)==0);assert(send(&p,0)==0);
 assert(send(&p,64)==(0x0Eu|(0xE0u<<8)|(0u<<16)|(64u<<24)));
 assert(send(&p,0xFA)==(0x0Fu|(0xFAu<<8)));
 assert(send(&p,0xFC)==(0x0Fu|(0xFCu<<8)));
 assert(p.frames==7&&p.realtime==3);
 puts("PASS TRS 31250 baud parser: running status, realtime, MIDI channel, SysEx and system-common");return 0;
}
