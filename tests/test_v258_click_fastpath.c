/* Exact equivalence of the CORE2 final-clamp fast path, all click signs. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

static int32_t clamp15(int32_t x){return x<-32768?-32768:x>32767?32767:x;}
static void original(int16_t dryl,int16_t dryr,uint16_t gain,int16_t click,int16_t *out){
 int32_t l=clamp15(((int32_t)dryl*gain)/32768);
 int32_t r=clamp15(((int32_t)dryr*gain)/32768);
 out[0]=(int16_t)clamp15(l+click);
 out[1]=(int16_t)clamp15(r+click);
}
static void optimized(int16_t dryl,int16_t dryr,uint16_t gain,int16_t click,int16_t *out){
 int32_t l=clamp15(((int32_t)dryl*gain)/32768);
 int32_t r=clamp15(((int32_t)dryr*gain)/32768);
 if(click){
  out[0]=(int16_t)clamp15(l+click);
  out[1]=(int16_t)clamp15(r+click);
 }else{
  out[0]=(int16_t)l;
  out[1]=(int16_t)r;
 }
}
static uint32_t rng=0xd7e923a1u;
static uint32_t next(void){rng=rng*1664525u+1013904223u;return rng;}
int main(void){
 const int16_t edges[]={-32768,-32767,-1,0,1,32766,32767};
 const uint16_t gains[]={0,1,16384,32767};
 const int16_t clicks[]={-32768,-32767,-1,0,1,32766,32767};
 int16_t a[2],b[2];uint32_t cases=0u,zero=0u;
 for(uint32_t i=0;i<7;i++)for(uint32_t j=0;j<7;j++)
  for(uint32_t k=0;k<4;k++)for(uint32_t q=0;q<7;q++){
   original(edges[i],edges[j],gains[k],clicks[q],a);
   optimized(edges[i],edges[j],gains[k],clicks[q],b);
   assert(a[0]==b[0]&&a[1]==b[1]);cases++;if(!clicks[q])zero++;
  }
 for(uint32_t i=0;i<2000000u;i++){
  int16_t l=(int16_t)next(),r=(int16_t)next();
  uint16_t gain=(uint16_t)(next()&32767u);
  int16_t click=(i%13u==0u)?(int16_t)next():0;
  original(l,r,gain,click,a);
  optimized(l,r,gain,click,b);
  assert(a[0]==b[0]&&a[1]==b[1]);cases++;if(!click)zero++;
 }
 printf("PASS V2.0.58 CORE2 final click/gain: %u sample-exact cases (%u zero-click)\n",cases,zero);
}
