/* Exact CORE2 post-gain PCM safety with all 65,536 possible gain values. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

static int32_t clamp15(int32_t v){return v<-32768?-32768:v>32767?32767:v;}
static int32_t legacy(int16_t pcm,uint16_t gain){
 return clamp15(((int32_t)pcm*(int32_t)gain)/32768);
}
static int32_t fast(int16_t pcm,uint16_t gain){
 int32_t x=((int32_t)pcm*(int32_t)gain)/32768;
 return gain<=32768u?x:clamp15(x);
}
static uint32_t rand_state=0x74ac319du;
static uint32_t next(void){rand_state^=rand_state<<13;rand_state^=rand_state>>17;rand_state^=rand_state<<5;return rand_state;}

int main(void){
 const int16_t edges[]={-32768,-32767,-16384,-1,0,1,16384,32766,32767};
 uint32_t cases=0,skipped=0;
 for(uint32_t g=0;g<65536u;g++){
  for(uint32_t k=0;k<sizeof(edges)/sizeof(edges[0]);k++){
   int32_t a=legacy(edges[k],(uint16_t)g),b=fast(edges[k],(uint16_t)g);
   assert(a==b);cases++;if(g<=32768u)skipped++;
  }
 }
 for(uint32_t i=0;i<2000000u;i++){
  int16_t pcm=(int16_t)next();
  uint16_t gain=(i%3u==0u)?(uint16_t)next():(uint16_t)(next()&32767u);
  int32_t a=legacy(pcm,gain),b=fast(pcm,gain);
  assert(a==b);cases++;if(gain<=32768u)skipped++;
 }
 printf("PASS V2.0.58: %u gain samples bit-exact (%u clamps safely skipped)\n",cases,skipped);
 return 0;
}
