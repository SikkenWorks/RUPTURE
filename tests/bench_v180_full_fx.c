#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"
static struct etch_native_fx fx;
static double stamp(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+(double)t.tv_nsec*1e-9;}
int main(void){
 struct etch_fx_settings c;etch_fx_defaults(&c);
 c.comp_ratio=26000;c.comp_threshold=26000;c.ring_mix=14000;c.eq_low=23000;c.eq_mid=24000;c.eq_high=27000;
 c.trem_mix=27000;c.trem_depth=22000;c.phaser_mix=27000;c.phaser_feedback=14000;
 c.flanger_mix=26000;c.flanger_feedback=15000;c.decim_mix=24000;
 c.tape_mix=24000;c.reverse_mix=18000;c.vibrato_mix=21000;c.rotary_mix=23000;c.wah_mix=24000;
 c.reverb_decay=26000;c.reverb_size=25000;
 const int N=500000;
 for(int typ=0;typ<3;typ++){
  etch_fx_reset(&fx);etch_fx_set_reverb_type(&c,(uint8_t)typ);
  int16_t l=0,r=0;uint32_t sum=0;
  double start=stamp();
  for(int i=0;i<N;i++){
   int16_t v=(int16_t)((((uint32_t)i*713u)^(i>>6))%24000u-12000);
   etch_fx_process_stereo_masked(&fx,v,24000,25000,30000,&c,0,&l,&r);
   sum+=(uint16_t)l+(uint16_t)r;
  }
  printf("engine %d, ns/sample %.1f, digest %u\n",typ,(stamp()-start)*1e9/N,sum);
 }
}
