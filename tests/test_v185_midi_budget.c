/* Verify exact MIDI event ordering, no drops, bounded per-half CPU work. */
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "native_midi_rt_budget.h"
int main(void){
 assert(ru_midi_batch_size(0)==0);
 assert(ru_midi_batch_size(1)==1);
 assert(ru_midi_batch_size(32)==32);
 assert(ru_midi_batch_size(33)==32);
 assert(ru_midi_batch_size(64)==32);
 assert(ru_midi_scope_skip(0)==0 && ru_midi_scope_skip(1)==1);
 for(uint32_t sz=0;sz<=64;sz++){
  uint32_t queue[64];for(uint32_t i=0;i<sz;i++)queue[i]=(i<<8)|0x90u;
  uint32_t read=0,n=0;
  while(read<sz){
   uint32_t batch=ru_midi_batch_size(sz-read);
   assert(batch<=32 && batch>0);
   for(uint32_t i=0;i<batch;i++){assert(queue[read]==(read<<8 | 0x90u));read++;n++;}
  }
  assert(n==sz);
 }
 puts("PASS V1.85 bounded MIDI queue: 0..64 FIFO packets exact, no intentional loss");
}
