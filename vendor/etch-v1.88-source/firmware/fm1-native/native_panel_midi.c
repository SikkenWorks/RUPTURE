/* SPDX-License-Identifier: GPL-3.0-only */
#include "native_panel_midi.h"
static uint8_t panel_note(uint32_t key,int8_t octave){
 int n=53+(int)key+12*(int)octave;
 return (uint8_t)(n<0?0:n>127?127:n);
}
void etch_panel_midi_reset(struct etch_panel_midi *m){
 if(!m)return;
 for(uint8_t k=0;k<27;k++){m->active[k]=0;m->note[k]=0;}
 m->cached_keys=0u;m->cached_octave=0;
 m->cached_connected=0u;m->cached_valid=0u;m->retry_pending=0u;
}
void etch_panel_midi_update(struct etch_panel_midi *m,uint32_t keys,int8_t octave,
                           uint8_t connected,etch_panel_emit emit,void *context){
 if(!m)return;
 /* A disconnected panel never has pending USB notes. Avoid repeating a
  * 54-entry clear at each DMA half once the disconnect is acknowledged. */
 if(!connected){
  if(!m->cached_valid || m->cached_connected){
   etch_panel_midi_reset(m);m->cached_valid=1u;
  }
  return;
 }
 if(!emit)return;
 keys &= 0x7ffffffu;
 if(m->cached_valid && m->cached_connected && !m->retry_pending &&
    m->cached_keys==keys && m->cached_octave==octave)return;
 m->cached_valid=1u;m->cached_connected=1u;
 m->cached_keys=keys;m->cached_octave=octave;m->retry_pending=0u;
 /* Always send releases before note-ons, including octave changes while held.
  * If the USB queue is full, an unsent release remains pending for retry. */
 for(uint8_t k=0;k<27;k++){
  if(!m->active[k])continue;
  uint8_t next=panel_note(k,octave);
  if(!(keys&(1u<<k)) || next!=m->note[k]){
   uint32_t packet=0x08u|(uint32_t)0x80u<<8|(uint32_t)m->note[k]<<16;
   if(emit(packet,context))m->active[k]=0;else m->retry_pending=1u;
  }
 }
 for(uint8_t k=0;k<27;k++){
  if(!(keys&(1u<<k))||m->active[k])continue;
  uint8_t note=panel_note(k,octave);
  uint32_t packet=0x09u|(uint32_t)0x90u<<8|(uint32_t)note<<16|(uint32_t)127u<<24;
  if(emit(packet,context)){m->active[k]=1;m->note[k]=note;}else m->retry_pending=1u;
 }
}
