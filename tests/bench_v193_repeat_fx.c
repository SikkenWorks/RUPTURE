/* Host-only post-mix microbenchmark; never use as FM1 DMA evidence. */
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"

static struct etch_native_fx fx;
static double timer_ns(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC_RAW,&t);
 return (double)t.tv_sec*1000000000.+t.tv_nsec;}
static uint32_t sample_case(uint8_t type,uint8_t count){
 struct etch_fx_settings c;etch_fx_defaults(&c);etch_fx_reset(&fx);
 c.folder_fold=24000u;c.folder_mix=22000u;
 c.wah_mix=24000u;c.wah_sens=20000u;
 c.comp_ratio=22000u;c.comp_threshold=17000u;
 struct rupture_fx_chain chain;ru_chain_clear(&chain);
 for(uint8_t n=0u;n<count;n++)ru_chain_insert(&chain,n,type);
 uint32_t hash=0u;
 for(uint32_t n=0u;n<300000u;n++){
  int16_t l=0,r=0;
  int16_t sample=(int16_t)((((uint32_t)n*713u)^((uint32_t)n>>6u))%24000u-12000);
  rupture_fx_process_ordered(&fx,sample,24000u,25000u,30000u,24000u,
   &c,&chain,0u,&l,&r);
  hash+=(uint16_t)l+(uint16_t)r;
 }
 return hash;
}
int main(void){
 struct {const char*name;uint8_t type,count;} cases[]={
  {"8 EMPTY",RU_FX_REVERB,0},
  {"1 REVERB",RU_FX_REVERB,1},
  {"8 REVERB",RU_FX_REVERB,8},
  {"1 DELAY",RU_FX_DELAY,1},
  {"8 DELAY",RU_FX_DELAY,8},
  {"1 DISTORTION",RU_FX_DISTORTION,1},
  {"8 DISTORTION",RU_FX_DISTORTION,8},
  {"1 WAH",RU_FX_WAH,1},
  {"8 WAH",RU_FX_WAH,8},
 };
 for(unsigned i=0u;i<sizeof(cases)/sizeof(cases[0]);i++){
  double t=timer_ns();uint32_t hash=sample_case(cases[i].type,cases[i].count);
  printf("%s: %.2f ns/sample; hash %u\n",cases[i].name,
    (timer_ns()-t)/300000.0,hash);
 }
 return 0;
}
