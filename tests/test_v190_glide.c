#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_mono_policy.h"
static void check(uint32_t from,uint32_t to,uint8_t curve){
 uint32_t at=from,previous=at;
 for(uint32_t remaining=512u;remaining;remaining--){
  at=rupture_glide_next_curve(at,to,remaining,curve);
  if(to>from)assert(at>=previous&&at<=to);
  else assert(at<=previous&&at>=to);
  previous=at;
 }
 assert(at==to);
}
int main(void){
 check(100000u,1000000u,0u);check(1000000u,100000u,0u);
 check(100000u,1000000u,1u);check(1000000u,100000u,1u);
 assert(rupture_glide_next_curve(1000u,9000u,100u,1u)>
        rupture_glide_next_curve(1000u,9000u,100u,0u));
 puts("PASS V1.90 sample-accurate linear/exponential bidirectional glide");
}
