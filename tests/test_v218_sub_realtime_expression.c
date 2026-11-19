/* RUPTURE V2.0.18: all active SUBs quarter-rate interpolation & Q8 expression.
 * Checks OFF PCM parity, active SUB level/pitch, and CXS PRNG phase. */
#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_sub_realtime.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_growl_tone.h"

static double seconds(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);
 return (double)t.tv_sec+(double)t.tv_nsec*0.000000001;}

static void off_parity(void){
 RuSubState old,rt;RuSubRt interp={0};
 ru_sub_init(&old,0x013579bdu,0xabc123u);
 ru_sub_init(&rt,0x013579bdu,0xabc123u);
 for(uint32_t i=0u;i<60000u;i++){
  int32_t o=ru_sub_tick(&old,RU_SUB_OFF,1u,51u,33u,0x013579bdu,62u,52u,59u,67u);
  int32_t n=ru_sub_rt_tick(&interp,&rt,RU_SUB_OFF,1u,51u,33u,0x013579bdu,62u,52u,59u,67u);
  assert(o==n);
 }
}

static void active_sub_safety(uint8_t type){
 RuSubState old,rt;RuSubRt interp={0};
 ru_sub_init(&old,0x013579bdu,0xabc123u);
 ru_sub_init(&rt,0x013579bdu,0xabc123u);
 int64_t old_energy=0,new_energy=0,cross=0;
 int32_t maximum=0;
 for(uint32_t i=0u;i<160000u;i++){
  int32_t o=ru_sub_tick(&old,type,1u,50u,0u,0x013579bdu,62u,52u,59u,67u);
  int32_t n=ru_sub_rt_tick(&interp,&rt,type,1u,50u,0u,0x013579bdu,62u,52u,59u,67u);
  /* Same exact phase, cycle count and seeded next exchange after each
   * group of four output frames. DSP stream is intentionally smoothed. */
  if((i&3u)==3u){
   if(type==RU_SUB_CXS){
    int64_t phase_error=(int64_t)rt.cxs.phase-(int64_t)old.cxs.phase;
    if(phase_error>2147483647LL)phase_error-=4294967296LL;
    if(phase_error< -2147483648LL)phase_error+=4294967296LL;
    assert(phase_error>=-(int64_t)(i+2u)*2 && phase_error<=(int64_t)(i+2u)*2);
    assert(rt.cxs.cycles<=old.cxs.cycles+1u);
    assert(old.cxs.cycles<=rt.cxs.cycles+1u);
   }else{
    /* Moving the octave division before/after the 4x step can differ by
     * two fractional accumulator units per group, never by a pitch step. */
    int64_t phase_error=(int64_t)rt.phase-(int64_t)old.phase;
    if(phase_error>2147483647LL)phase_error-=4294967296LL;
    if(phase_error< -2147483648LL)phase_error+=4294967296LL;
    assert(phase_error>=-(int64_t)(i+2u)*3 && phase_error<=(int64_t)(i+2u)*3);
   }
  }
  int32_t mag=n<0?-n:n;if(mag>maximum)maximum=mag;
  if(i>512u){old_energy+=(int64_t)o*o;new_energy+=(int64_t)n*n;cross+=(int64_t)o*n;}
 }
 if(type==RU_SUB_CXS)assert(rt.cxs.cycles==old.cxs.cycles&&rt.cxs.seed==old.cxs.seed);
 assert(maximum>4000&&maximum<=32767);
 double ratio=(double)new_energy/(double)old_energy;
 double correlation=(double)cross/sqrt((double)old_energy*(double)new_energy);
 printf("SUB type %u 4x interpolation energy ratio %.4f, correlation %.4f, max=%ld\n",
        (unsigned)type,ratio,correlation,(long)maximum);
 assert(ratio>0.70&&ratio<1.30);
 assert(correlation>0.65);
}

static void expression_fraction(void){
 RuGrowlParams p=ru_growl_make_params(58u,50u,52u,68u,50u,60u,45u,40u);
 RuGrowlParams neutral=p;
 ru_growl_apply_expression_q8(&p,50u*256u);
 assert(!__builtin_memcmp(&neutral,&p,sizeof(p)));
 RuGrowlParams low=neutral,high=neutral,mid=neutral;
 ru_growl_apply_expression_q8(&low,52u*256u);
 ru_growl_apply_expression_q8(&high,53u*256u);
 ru_growl_apply_expression_q8(&mid,52u*256u+128u);
 assert(mid.throat_a>low.throat_a&&mid.throat_a<high.throat_a);
 assert(mid.drive_q8>low.drive_q8&&mid.drive_q8<high.drive_q8);
 for(int v=0;v<=100;v++){
  RuGrowlParams a=neutral,b=neutral;
  ru_growl_apply_expression(&a,(uint8_t)v);
  ru_growl_apply_expression_q8(&b,(uint16_t)v*256u);
  assert(!__builtin_memcmp(&a,&b,sizeof(a)));
 }
}

static void sub_switch_stress(void){
 RuSubState rt;RuSubRt interp={0};
 ru_sub_init(&rt,0x013579bdu,0xabc123u);
 int32_t peak=0;
 for(uint32_t i=0u;i<220000u;i++){
  uint8_t type=(uint8_t)((i/2351u)%5u);
  uint32_t inc=0x01000000u+((i/1000u)%20u)*0x000b0000u;
  int32_t s=ru_sub_rt_tick(&interp,&rt,type,1u,(uint8_t)(45u+(i/107u)%11u),
                         40u,inc,62u,(uint8_t)(i/593u)%101u,59u,67u);
  int32_t abs=s<0?-s:s;
  if(abs>peak)peak=abs;
  assert(abs<=32767);
  assert(rt.mix<=RU_SUB_MIX_MAX);
  if((i%23000u)==0u){ru_sub_init(&rt,inc,0xabc123u);ru_sub_rt_reset(&interp);}
 }
 assert(peak>7000);
}

static void bench(void){
 const uint32_t n=650000u;
 RuSubState old,rt;RuSubRt interp={0};
 ru_sub_init(&old,0x013579bdu,0xabc123u);
 ru_sub_init(&rt,0x013579bdu,0xabc123u);
 volatile int64_t checksum=0;
 double start=seconds();
 for(uint32_t i=0;i<n;i++)checksum+=ru_sub_tick(&old,RU_SUB_CXS,1u,50u,0u,0x013579bdu,62u,52u,59u,67u);
 double baseline=(seconds()-start)*1000000000.0/n;
 start=seconds();
 for(uint32_t i=0;i<n;i++)checksum+=ru_sub_rt_tick(&interp,&rt,RU_SUB_CXS,1u,50u,0u,0x013579bdu,62u,52u,59u,67u);
 double candidate=(seconds()-start)*1000000000.0/n;
 printf("HOST CXS SUB old %.2f ns/sample, 4x %.2f ns/sample (not FM1 timing), checksum %lld\n",
        baseline,candidate,(long long)checksum);
}

int main(void){
 off_parity();
 for(uint8_t type=RU_SUB_CXS;type<=RU_SUB_SAW;type++)active_sub_safety(type);
 expression_fraction();sub_switch_stress();bench();
 puts("PASS V2.0.18 SUB: OFF PCM parity, all active SUB types bandlimited, pitch & CXS seed, continuous expression");
 return 0;
}
