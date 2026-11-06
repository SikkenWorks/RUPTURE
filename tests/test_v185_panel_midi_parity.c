/* V1.85: 120k randomized state/USB-backpressure steps compare new & original
 * panel MIDI packet order, pitch, and pending Note OFF retries. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "native_panel_midi.h"
void reference_panel_reset(struct etch_panel_midi*);
void reference_panel_update(struct etch_panel_midi*,uint32_t,int8_t,uint8_t,etch_panel_emit,void*);
struct sink {uint32_t packets[64];unsigned count,capacity;};
static int emit(uint32_t pkt,void *arg){struct sink *s=(struct sink*)arg;if(s->count>=s->capacity)return 0;s->packets[s->count++]=pkt;return 1;}
static uint32_t rand_state=0x9ab27c31u;
static uint32_t next(void){rand_state^=rand_state<<13;rand_state^=rand_state>>17;rand_state^=rand_state<<5;return rand_state;}
int main(void){
 struct etch_panel_midi base={0},opt={0};reference_panel_reset(&base);etch_panel_midi_reset(&opt);
 uint32_t keys=0u;int8_t octave=0;uint8_t connected=0;
 uint32_t skipped=0,events=0,full=0;
 for(uint32_t i=0;i<120000u;i++){
  uint32_t n=next();
  if((n&31u)==0u)connected^=1u;
  if((n&15u)==0u)keys^=1u<<((n>>16)%27u);
  if((n&127u)==2u)keys=0;
  if((n&127u)==3u)keys=0x7ffffffu;
  if((n&127u)==6u)octave=(int8_t)((n>>12)%8u-4);
  if((n&63u)==7u)keys^=1u<<(n%27u);
  unsigned capacity=(n&7u)==4u?0u:(n&63u)==1u?1u:64u;
  struct sink sb={.capacity=capacity},so={.capacity=capacity};
  reference_panel_update(&base,keys,octave,connected,emit,&sb);
  etch_panel_midi_update(&opt,keys,octave,connected,emit,&so);
  if(sb.count!=so.count || memcmp(sb.packets,so.packets,sb.count*sizeof(uint32_t)) ||
     memcmp(base.active,opt.active,27)||memcmp(base.note,opt.note,27)){
   fprintf(stderr,"V1.85 divergence at step %u base=%u opt=%u key=%x octave=%d conn=%u\n",i,sb.count,so.count,keys,octave,connected);
   return 1;
  }
  if(so.count==0 && connected && opt.cached_valid && !opt.retry_pending &&
      opt.cached_keys==keys && opt.cached_octave==octave)skipped++;
  if(capacity==0)full++;
  events+=so.count;
 }
 printf("PASS V1.85 panel MIDI 120000 states; packets=%u idle_or_quiet=%u blocked_tx_cases=%u\n",events,skipped,full);
}
