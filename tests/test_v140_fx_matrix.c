/* RUPTURE FX V1.40 sound-parity and realtime bypass regression.
   Compile identical harness against V1.39 and V1.40 native_fx.c and
   compare PCM checksums. Effect activation is the only changed behavior. */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx_bypass_v181.h"
static struct etch_native_fx fx;
static uint32_t hashfn(uint32_t h, uint16_t v){return (h ^ (uint8_t)v)*16777619u ^ ((uint32_t)(v>>8)*16777619u);}
int main(int argc,char **argv){
 unsigned n=argc>1?(unsigned)atoi(argv[1]):60000u;
 for(int scenario=0;scenario<12;scenario++){
  struct etch_fx_settings c;etch_fx_reset(&fx);etch_fx_defaults(&c);
  uint16_t ch=0,de=0,re=0;
  uint8_t fuzz=0,tilt=0,sand=0;
  if(scenario==1||scenario==8||scenario==9||scenario==10||scenario==11)fuzz=1;
  if(scenario==2||scenario==8||scenario==10)re=14000;
  if(scenario==3||scenario==8||scenario==11)de=14000;
  if(scenario==4||scenario==8||scenario==9)ch=14000;
  if(scenario==5||scenario==10)c.comp_ratio=14000,c.comp_threshold=18000;
  if(scenario==6||scenario==9)tilt=1;
  if(scenario==7||scenario==11)sand=1;
  rupture_fuzz_controls(55,75,65,80,fuzz);
  rupture_tilt_controls(45,50,85,50,tilt);
  rupture_sandpaper_controls(60,40,45,80,sand);
  uint32_t h=2166136261u;
  int64_t acc=0;
  for(unsigned i=0;i<n;i++){
   /* Nontrivial excitation: pseudo-random bounded signal + bass-like swing */
   uint32_t x=(i*1103515245u+12345u) ^ ((i/101u)*1315423911u);
   int16_t in=(int16_t)((int32_t)(x%32001u)-16000);
   if(scenario==1 && i==n/2u)rupture_fuzz_controls(55,75,65,80,0);
   int16_t l=0,r=0;
   etch_fx_process_stereo_masked(&fx,in,ch,de,re,&c,0,&l,&r);
   h=hashfn(h,(uint16_t)l);h=hashfn(h,(uint16_t)r);
   acc+=(int32_t)l-(int32_t)r;
  }
  printf("%d %u %lld\n",scenario,h,(long long)acc);
 }
 return 0;
}
