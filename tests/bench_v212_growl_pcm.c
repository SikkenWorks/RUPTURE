/* HOST benchmark only. Do not interpret as real FM1 DMA deadline proof.
 * Compile twice with -I old/new fm1-native dirs; add -DNEW_GROWL for V2.0.12. */
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include "rupture_growl_waves.h"
#include "rupture_growl_tone.h"

static uint64_t ns_now(void){
 struct timespec ts;clock_gettime(CLOCK_MONOTONIC,&ts);
 return (uint64_t)ts.tv_sec*1000000000u+(uint64_t)ts.tv_nsec;
}
int main(void){
 const uint32_t nmax=5000000u;
 const uint32_t inc=10500000u; /* approx A2 on FM1 */
 uint64_t start=ns_now();
 uint32_t hash=2166136261u;
 RuGrowlState st={0};
 uint32_t phase=0;
 uint8_t bright=50u,edge=50u;
 for(uint32_t n=0;n<nmax;n++){
  phase+=inc;
  if(!(n&127u)){
   bright=(uint8_t)((n/128u)%101u);
   edge=(uint8_t)(((n/128u)*37u)%101u);
  }
  int32_t carrier=ru_growl_read(phase,inc);
#ifdef NEW_GROWL
  /* Exercise the EXACT cached-coeff kernel used by render_internal(). */
  static RuGrowlParams p;
  if(!(n&127u))
   p=ru_growl_make_params(bright,edge,67u,68u,74u,55u,66u,42u);
  int32_t v=ru_growl_process(&st,carrier,&p);
#else
  int32_t v=rupture_growl_tone(&st,carrier,bright,edge,67u,68u,
                              74u,55u,66u,42u);
#endif
  hash=(hash^(uint16_t)v)*16777619u;
 }
 uint64_t elapsed=ns_now()-start;
 printf("%s %llu ns / %u samples = %.2f ns/sample, checksum %u\n",
#ifdef NEW_GROWL
        "V2.0.12 block cached",
#else
        "V2.0.11 per-sample",
#endif
        (unsigned long long)elapsed,nmax,(double)elapsed/nmax,hash);
 return 0;
}
