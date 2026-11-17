/* CXS note-on DMA optimization: sample-exact vs original first exchange. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_cxs_q15.c"

/* Recreate V2.0.4's exact first exchange, with its expensive divisions. */
static void original_init(RuptureCXSQ15 *s,uint32_t increment){
 memset(s,0,sizeof(*s));
 s->increment=increment;s->seed=0x76fe8217u;
 s->pressure=58u;s->balance=50u;s->memory=52u;s->density=68u;s->edge=50u;
 for(uint32_t i=0;i<8u;i++)s->durations[i]=4096u;
 exchange(s);
}
int main(void){
 for(uint32_t note=0u;note<128u;note++){
  /* Cover low, normal and extreme note increments. */
  uint32_t increment=0x03800000u+note*0x00a71937u;
  RuptureCXSQ15 old,new_state;
  original_init(&old,increment);
  rupture_q15_init(&new_state,increment);
  assert(memcmp(&old,&new_state,sizeof(old))==0);
  if(note&1u){old.seed^=0x61230af5u;new_state.seed^=0x61230af5u;}
  else if(note&2u){old.seed^=0xa9521873u;new_state.seed^=0xa9521873u;}
  for(uint32_t sample=0u;sample<2048u;sample++){
   if((sample&255u)==0u){
    uint8_t p=(uint8_t)(note%101u);
    rupture_q15_params(&old,p,(uint8_t)((p+13u)%101u),(uint8_t)((p+31u)%101u),(uint8_t)((p+61u)%101u));
    rupture_q15_params(&new_state,p,(uint8_t)((p+13u)%101u),(uint8_t)((p+31u)%101u),(uint8_t)((p+61u)%101u));
   }
   assert(rupture_q15_sample(&old)==rupture_q15_sample(&new_state));
  }
 }
 puts("PASS V2.0.5: carrier/SUB CXS startup is byte- and 262144-sample-identical; no exchange division on note-on");
 return 0;
}
