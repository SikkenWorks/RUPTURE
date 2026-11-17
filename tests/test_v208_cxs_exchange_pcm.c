/* V2.0.8: compare the exact old 64-bit CXS exchange against the new
 * bounded 32-bit implementation, including rapid wraps, both SUB seeds,
 * live knob edits and all edge interpolation values. */
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_cxs_q15.h"
void old_rupture_q15_init(RuptureCXSQ15*,uint32_t);
void old_rupture_q15_params(RuptureCXSQ15*,uint8_t,uint8_t,uint8_t,uint8_t);
int16_t old_rupture_q15_sample(RuptureCXSQ15*);

static uint32_t next(uint32_t *seed){
 uint32_t x=*seed;x^=x<<13;x^=x>>17;x^=x<<5;*seed=x;return x;
}
int main(void){
 uint32_t rng=0x4aa53b79u;
 uint64_t samples=0,cycles=0;
 for(unsigned scenario=0;scenario<32u;scenario++){
  RuptureCXSQ15 old,now;
  uint32_t base=0x00200000u+(next(&rng)&0x1fffffffu);
  old_rupture_q15_init(&old,base);
  rupture_q15_init(&now,base);
  old.seed=now.seed=next(&rng);
  old.edge=now.edge=(uint8_t)((scenario&1u)?50u:next(&rng)%101u);
  for(unsigned n=0;n<11000u;n++){
   if((n%137u)==0u){
    uint8_t a=(uint8_t)(next(&rng)%101u),b=(uint8_t)(next(&rng)%101u);
    uint8_t c=(uint8_t)(next(&rng)%101u),d=(uint8_t)(next(&rng)%101u);
    old_rupture_q15_params(&old,a,b,c,d);
    rupture_q15_params(&now,a,b,c,d);
   }
   if((n%503u)==0u){
    old.increment=now.increment=0x00200000u+(next(&rng)&0x7fffffffu);
   }
   int16_t a=old_rupture_q15_sample(&old);
   int16_t b=rupture_q15_sample(&now);
   const size_t state_bytes=offsetof(RuptureCXSQ15,edge)+sizeof(old.edge);
   if(a!=b||memcmp(&old,&now,state_bytes)!=0){
    fprintf(stderr,"CXS PCM/state mismatch scenario=%u sample=%u old=%d new=%d\n",scenario,n,a,b);
    for(unsigned j=0;j<state_bytes;j++)if(((uint8_t*)&old)[j]!=((uint8_t*)&now)[j])
     fprintf(stderr," differing state byte %u: %u vs %u\n",j,((uint8_t*)&old)[j],((uint8_t*)&now)[j]);
    return 1;
   }
   samples++;
  }
  cycles+=now.cycles-1u; /* init begins at cycle 1: count actual wraps only */
 }
 assert(samples==352000u&&cycles>5000u);
 printf("PASS V2.0.8 old/new CXS sample + complete state parity: %llu samples, %llu cycles\n",
  (unsigned long long)samples,(unsigned long long)cycles);
 return 0;
}
