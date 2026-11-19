/* Scope preparation is optional under pressure. Validate the optimization
 * never skips a scope frame that V2.0.29 would actually publish. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_audio_timing.h"

int main(void){
 uint32_t cases=0u,skipped=0u;
 const uint32_t deadline=128u*544u;
 for(uint32_t pressure=0u;pressure<256u;pressure++){
  for(uint32_t backlog=0u;backlog<2u;backlog++){
   for(uint32_t ticks=0u;ticks<=deadline+2000u;ticks+=73u){
    uint8_t previous=(uint8_t)(!backlog&&etch_audio_scope_may_capture(
                                   ticks,deadline,(uint8_t)pressure));
    uint8_t prepare=(uint8_t)(!backlog&&pressure<16u);
    uint8_t now=(uint8_t)(prepare&&etch_audio_scope_may_capture(
                                   ticks,deadline,(uint8_t)pressure));
    assert(previous==now);
    assert(!now||prepare);
    if(!prepare)skipped++;
    cases++;
   }
  }
 }
 assert(skipped>cases/2u);
 printf("PASS V2.0.30 optional scope: %u publish decisions identical, %u unused mono preparations elided\n",cases,skipped);
 return 0;
}
