#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_fx_chain.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_drive_softclip.h"

static struct etch_native_fx fx;
static double now_ns(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC_RAW,&t);
 return (double)t.tv_sec*1000000000.+t.tv_nsec;}
static uint32_t run(unsigned mode,int N){
 struct etch_fx_settings settings;etch_fx_defaults(&settings);
 settings.comp_ratio=26000u;settings.comp_threshold=24000u;
 settings.reverb_size=23000u;settings.reverb_decay=25000u;
 settings.delay_feedback=18000u;settings.delay_time=16000u;
 settings.chorus_rate=11000u;settings.chorus_depth=13000u;
 etch_fx_reset(&fx);
 rupture_tilt_controls(75u,50u,65u,50u,(uint8_t)(mode>=2u&&mode!=5u));
 rupture_fuzz_controls(25u,55u,30u,50u,(uint8_t)(mode>=2u&&mode!=5u));
 rupture_sandpaper_controls(50u,50u,55u,50u,(uint8_t)(mode>=2u&&mode!=5u));
 struct rupture_fx_chain chain;ru_chain_clear(&chain);
 if(mode==1u){ru_chain_insert(&chain,0u,RU_FX_REVERB);}
 if(mode==2u||mode==4u){
  static const uint8_t order[8]={RU_FX_COMP,RU_FX_TILT,RU_FX_FUZZ,RU_FX_SAND,
    RU_FX_CHORUS,RU_FX_DELAY,RU_FX_REVERB,RU_FX_MOSFET};
  for(unsigned i=0;i<(mode==4u?7u:8u);i++)ru_chain_insert(&chain,(uint8_t)i,order[i]);
 }
 if(mode==5u)ru_chain_insert(&chain,0u,RU_FX_MOSFET);
 uint32_t digest=0u;
 for(int i=0;i<N;i++){
  int16_t l=0,r=0;
  int16_t x=(int16_t)((((uint32_t)i*713u)^((uint32_t)i>>6))%24000u-12000);
  if(mode==3u||mode==6u){if(mode==3u)x=(int16_t)etch_drive_softclip(x,24000u);
   etch_fx_process_stereo_masked(&fx,x,24000,25000,30000,&settings,0,&l,&r);}
  else rupture_fx_process_ordered(&fx,x,24000,25000,30000,24000,&settings,&chain,0,&l,&r);
  digest+=(uint16_t)l+(uint16_t)r;
 }
 return digest;
}
int main(void){
 const int N=300000;
 for(unsigned mode=0;mode<7;mode++){
  double start=now_ns();uint32_t result=run(mode,N);double elapsed=now_ns()-start;
  printf("%s: %.2f ns/sample; checksum %u\n",
    mode==0?"8 EMPTY":mode==1?"REVERB ONLY":mode==2?"8 SLOT FX":
    mode==3?"V1.89 LEGACY+DRIVE":mode==4?"7 SLOT NO MOSFET":
    mode==5?"MOSFET ONLY":"V1.89 LEGACY FX",
    elapsed/N,result);
 }
}
