/* Reference PCM checksum: compile the SAME harness against original V1.68
 * and optimized V1.69 effects sources, compare full output. Changes within
 * the live compressor controls are intentional and must NOT alter samples. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"
static struct etch_native_fx state;
static uint64_t fnv(uint64_t h,uint16_t x){h=(h^(x&255u))*1099511628211ull;return (h^((x>>8)&255u))*1099511628211ull;}
int main(void){
 for(int scene=0;scene<16;scene++){
  struct etch_fx_settings c;
  etch_fx_defaults(&c);etch_fx_reset(&state);
  c.comp_ratio=(uint16_t)(scene==0?0:2500+scene*1900);
  c.comp_threshold=(uint16_t)(scene==0?0:(scene*1123)%32768);
  c.comp_attack=(uint16_t)((scene*1797)%32768);
  c.comp_release=(uint16_t)((scene*2597)%32768);
  if(scene&1)c.eq_low=22000,c.eq_mid=10000,c.eq_high=18000;
  if(scene&2)c.ring_mix=5000;
  if(scene&4)c.folder_mix=12000,c.folder_fold=19000;
  uint16_t ch=(scene&2)?12000:0, de=(scene&4)?9000:0,re=(scene&8)?10000:0;
  uint32_t bypass=0;uint64_t h=14695981039346656037ull;
  for(uint32_t i=0;i<50000u;i++){
   if(i%777u==0u){
    c.comp_ratio=(uint16_t)(((uint32_t)c.comp_ratio+2003u)%32768u);
    c.comp_attack=(uint16_t)(((uint32_t)c.comp_attack+1234u)%32768u);
    c.comp_release=(uint16_t)(((uint32_t)c.comp_release+2170u)%32768u);
    c.comp_threshold=(uint16_t)(((uint32_t)c.comp_threshold+393u)%32768u);
   }
   if(i%3901u==0u){c.comp_ratio=(uint16_t)((i/3901u)&1u?0u:20000u);}
   if(i%4999u==0u)bypass^=1u<<1;
   uint32_t r=(i*1103515245u+12345u)^((i/91u)*1013904223u);
   int16_t in=(int16_t)((int32_t)(r%50001u)-25000);
   int16_t l,rout;
   etch_fx_process_stereo_masked(&state,in,ch,de,re,&c,bypass,&l,&rout);
   h=fnv(fnv(h,(uint16_t)l),(uint16_t)rout);
  }
  printf("%02d %016llx\n",scene,(unsigned long long)h);
 }
 return 0;
}
