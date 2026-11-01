#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_shape.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_cxs_q15.h"
static int64_t render(uint8_t edge,uint8_t weight,uint8_t bite,uint8_t punch,uint32_t inc,uint32_t depth,uint8_t target){
 RuptureCXSQ15 c;rupture_q15_init(&c,inc);c.edge=edge;
 uint32_t lfo=0;int32_t low=0;int64_t hash=0;
 for(int i=0;i<16384;i++){
  lfo+=97392u; /* 1Hz */
  int32_t ph=(int32_t)(lfo>>16);
  int32_t tri=ph<32768?ph*2-32767:98303-ph*2;
  int32_t devi=(int32_t)(((int64_t)tri*depth*1940)/(32767*32767));
  c.increment=target?inc:(uint32_t)((int64_t)inc+((int64_t)inc*devi)/32768);
  int32_t x=rupture_q15_sample(&c);
  x=rupture_shape_sample(&low,x,weight,bite,punch,c.cycles);
  if(target && depth) x=(int32_t)(((int64_t)x*(32767-((int64_t)(tri+32767)*depth)/(2*32767)))/32767);
  assert(x>=-32767 && x<=32767);assert(rupture_q15_valid(&c));
  hash=(hash*131+(int64_t)x)&0x7fffffffffffffffLL;
 }
 return hash;
}
int main(void){
 uint32_t inc=(uint32_t)((4294967296ull*110u)/44100u);
 int64_t neutral=render(50,0,0,0,inc,0,0);
 assert(neutral!=render(50,0,0,0,inc,32767,0)); /* pitch LFO audible */
 assert(neutral!=render(50,0,0,0,inc,32767,1)); /* tremolo audible */
 assert(neutral!=render(10,0,0,0,inc,0,0));
 assert(neutral!=render(50,75,0,0,inc,0,0));
 assert(neutral!=render(50,0,75,0,inc,0,0));
 assert(neutral!=render(50,0,0,75,inc,0,0));
 printf("CXS LFO & four sound shapers: PASS\n");
}
