/* Compare the actual old V2.0.7 sub increment with the hardware-cheaper
 * V2.0.9 paired-register multiplication, including octave overflow. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "rupture_subosc.h"

static uint32_t legacy_sub_inc(uint32_t base,uint8_t octave,uint8_t detune,int32_t drift){
 uint64_t raw=base;
 if(octave==0u)base>>=2u;
 else if(octave==1u)base>>=1u;
 else if(octave==3u)raw<<=1u;
 int32_t cents=(int32_t)detune-50;
 int32_t signed_factor=65536+cents*38+drift;
 if(signed_factor<32768)signed_factor=32768;
 uint32_t factor=(uint32_t)signed_factor;
 if(factor>=131072u){
  uint64_t v=(octave==3u)?raw:(uint64_t)base;
  v=(v*factor)>>16;
  return (uint32_t)(v>UINT32_MAX?UINT32_MAX:v<1u?1u:v);
 }
 uint32_t fraction=factor&65535u;
 uint32_t lo=(base&65535u)*fraction;
 uint32_t middle=(base>>16)*fraction;
 uint32_t part=lo>>16;
 if(middle>UINT32_MAX-part)return UINT32_MAX;
 uint32_t result=middle+part;
 if(factor>=65536u){
  if(result>UINT32_MAX-base)return UINT32_MAX;
  result+=base;
 }
 if(octave==3u){
  if(result>0x7fffffffu)return UINT32_MAX;
  result=(result<<1u)|((lo>>15u)&1u);
 }
 return result?result:1u;
}
static uint32_t seed=0x98bd21e7u;
static uint32_t rnd(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static uint64_t checked;
static void compare(uint32_t base,uint8_t oct,uint8_t detune,int32_t drift){
 uint32_t old=legacy_sub_inc(base,oct,detune,drift);
 uint32_t now=ru_sub_inc(base,oct,detune,drift);
 if(now!=old){
  fprintf(stderr,"FAIL sub inc base=%u oct=%u detune=%u drift=%d old=%u now=%u\n",
   base,oct,detune,drift,old,now);exit(1);
 }
 ++checked;
}
int main(void){
 static const uint32_t bases[]={0u,1u,2u,32767u,65535u,65536u,
  0x7fffffffu,0x80000000u,0xefffffffu,0xfffffffeu,0xffffffffu};
 static const int32_t drifts[]={-100000,-768,-767,-256,-1,0,1,256,767,768,100000};
 for(unsigned b=0;b<sizeof(bases)/sizeof(bases[0]);b++)
  for(uint8_t o=0;o<5u;o++)
   for(unsigned d=0;d<256u;d++)
    for(unsigned k=0;k<sizeof(drifts)/sizeof(drifts[0]);k++)
     compare(bases[b],o,(uint8_t)d,drifts[k]);
 for(unsigned n=0;n<2000000u;n++){
  uint32_t b=rnd();
  uint8_t o=(uint8_t)(rnd()%5u);
  uint8_t d=(uint8_t)rnd();
  int32_t drift=(n&1u)?(int32_t)(rnd()%1537u)-768:(int32_t)(rnd()%200001u)-100000;
  compare(b,o,d,drift);
 }
 printf("PASS V2.0.9 legacy/new SUB inc bit exact: %llu cases, all octaves/detune/overflow\n",
  (unsigned long long)checked);
 return 0;
}
