#include <assert.h>
#include <stdint.h>
#include <stdio.h>

static uint32_t seed=0x72b9a1d5u;
static uint32_t random32(void){seed=seed*1664525u+1013904223u;return seed;}

static void check(int32_t dry,int32_t wet,int32_t fade){
 assert(dry>=-32768 && dry<=32767);
 assert(wet>=-32768 && wet<=32767);
 assert(fade>=0 && fade<=32767);
 int32_t difference=wet-dry;
 int64_t product=(int64_t)difference*fade;
 assert(product>=INT32_MIN && product<=INT32_MAX);
 int32_t new_delta=(difference*fade)/32767;
 int32_t old_delta=(int32_t)(product/32767);
 assert(new_delta==old_delta);
 assert(dry+new_delta==dry+old_delta);
}

int main(void){
 const int32_t values[]={-32768,-32767,-30000,-1,0,1,16384,30000,32766,32767};
 const int32_t fades[]={0,1,2,33,64,1024,16383,16384,32766,32767};
 unsigned long count=0u;
 for(unsigned d=0;d<sizeof(values)/sizeof(values[0]);d++)
 for(unsigned w=0;w<sizeof(values)/sizeof(values[0]);w++)
 for(unsigned f=0;f<sizeof(fades)/sizeof(fades[0]);f++){
  check(values[d],values[w],fades[f]);count++;
 }
 for(unsigned i=0;i<1000000u;i++){
  int32_t dry=(int16_t)(random32()>>16);
  int32_t wet=(int16_t)(random32()>>16);
  int32_t fade=(int32_t)(random32()>>17);
  check(dry,wet,fade);count++;
 }
 printf("PASS V2.0.28 FX fade: %lu stereo-delta cases exactly 64-bit equivalent\n",count);
 return 0;
}
