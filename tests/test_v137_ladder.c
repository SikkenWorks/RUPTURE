/* Exact regression against RUPTURE V1.36's previous ladder DSP.
 * Exercise cutoff/resonance extremes, random input, and dynamic sweeps.
 */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_ladder_fast.h"
static int32_t old_step(int32_t state[4], uint16_t cutoff,uint16_t resonance,int32_t input){
 uint32_t f=cutoff>8192u?8192u:cutoff;
 int32_t g=(int32_t)(f*3u);
 uint32_t res=resonance>32767u?32767u:resonance;
 int32_t k=(int32_t)((res*365u)/100u);
 int32_t x=input-(int32_t)(((int64_t)state[3]*k)>>15);
 if(x>32767)x=32767+(x-32767)/8;
 if(x< -32767)x=-32767+(x+32767)/8;
 if(x>49151)x=49151;
 if(x< -49151)x=-49151;
 for(uint32_t i=0;i<4;i++){
  int32_t z=state[i];
  int32_t delta=(int32_t)(((int64_t)(x-z)*g)>>15);
  z+=delta;
  if(z>32767)z=32767;
  if(z< -32767)z=-32767;
  state[i]=z;x=z;
 }
 return state[3];
}
static uint32_t seed=0xC1A55EEDu;
static uint32_t next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
int main(void){
 static const uint16_t fvalues[]={0,1,10,100,1024,4096,8192,65535};
 static const uint16_t rvalues[]={0,1,100,12000,24000,32767,65535};
 uint64_t count=0;
 for(unsigned fi=0;fi<sizeof(fvalues)/sizeof(*fvalues);fi++)
 for(unsigned ri=0;ri<sizeof(rvalues)/sizeof(*rvalues);ri++){
  int32_t before[4]={0},after[4]={0};
  for(int j=0;j<12000;j++){
   int32_t input=(int32_t)(next()%65536u)-32768;
   if((j%111)==0)input=32767;
   if((j%131)==0)input=-32768;
   int32_t a=old_step(before,fvalues[fi],rvalues[ri],input);
   int32_t b=rupture_ladder_fast_step(after,fvalues[fi],rvalues[ri],input);
   if(a!=b || memcmp(before,after,sizeof(before))!=0){
    fprintf(stderr,"FAIL cutoff %u res %u sample %d: %d != %d\n",fvalues[fi],rvalues[ri],j,a,b);return 2;
   }
   for(int k=0;k<4;k++)assert(after[k]>=-32767&&after[k]<=32767);
   count++;
  }
 }
 /* 300,000 samples of knob automation, including abrupt endpoints */
 int32_t astate[4]={0},bstate[4]={0};
 for(int j=0;j<300000;j++){
  uint16_t f=(uint16_t)(next()%8193u),r=(uint16_t)(next()%32768u);
  int32_t in=(int32_t)(next()%65536u)-32768;
  int32_t a=old_step(astate,f,r,in);
  int32_t b=rupture_ladder_fast_step(bstate,f,r,in);
  if(a!=b||memcmp(astate,bstate,sizeof(astate))){fprintf(stderr,"FAIL sweep %d\n",j);return 3;}
  count++;
 }
 /* Corrupted input is safely clipped instead of overflowing. */
 int32_t s[4]={0};
 for(int i=0;i<10000;i++){
  int32_t input=(i&1)?0x7fffffff:(int32_t)0x80000000u;
  int32_t result=rupture_ladder_fast_step(s,65535,65535,input);
  assert(result>=-32767 && result<=32767);
 }
 printf("PASS %llu exact old/new ladder samples, four-state integrity, bounds\n",(unsigned long long)count);
 return 0;
}
