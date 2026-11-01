/* SPDX-License-Identifier: GPL-3.0-only
 * RUPTURE: bounded TRS UART1 MIDI channel-message decoder.
 * USB-MIDI packet format; source=2 is assigned by the caller.
 * Running status, realtime interleaving, system-common and SysEx discard.
 * Does not write flash, allocate memory or change the trusted bootloader.
 */
#pragma once
#include <stdint.h>
typedef struct {
 uint8_t status, need, count, d1, sysex, common_skip;
 uint32_t frames, realtime, ignored;
} RuTrsParser;
static void ru_trs_reset(RuTrsParser *p){*p=(RuTrsParser){0};}
static int ru_trs_feed(RuTrsParser *p,uint8_t byte,uint32_t *out){
 if(byte>=0xF8u){
  /* MIDI clock/start/stop all use USB-MIDI CIN 0xF; unrecognized real-time
   * bytes may still be forwarded without affecting running status. */
  *out=0x0Fu|((uint32_t)byte<<8);p->realtime++;return 1;
 }
 if(byte&0x80u){
  if(byte>=0xF0u){
   p->status=0u;p->need=p->count=0u;
   if(byte==0xF0u){p->sysex=1u;p->common_skip=0u;}
   else {p->sysex=0u;p->common_skip=(uint8_t)(byte==0xF2u?2u:((byte==0xF1u||byte==0xF3u)?1u:0u));}
   return 0;
  }
  p->sysex=0u;p->common_skip=0u;
  p->status=byte;p->count=0u;
  p->need=(uint8_t)(((byte&0xF0u)==0xC0u||(byte&0xF0u)==0xD0u)?1u:2u);
  return 0;
 }
 if(p->sysex){p->ignored++;return 0;}
 if(p->common_skip){p->common_skip--;p->ignored++;return 0;}
 if(!p->status || !p->need){p->ignored++;return 0;}
 if(!p->count)p->d1=byte;
 p->count++;
 if(p->count<p->need)return 0;
 uint8_t cin=(uint8_t)(p->status>>4);
 *out=(uint32_t)cin|((uint32_t)p->status<<8)|((uint32_t)p->d1<<16)|((uint32_t)(p->need==2u?byte:0u)<<24);
 p->count=0u;p->frames++;return 1;
}
