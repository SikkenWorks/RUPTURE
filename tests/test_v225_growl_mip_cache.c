#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_growl_mip_cache.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_growl_tone.h"

static uint32_t pitch_at(uint8_t scenario,uint32_t frame){
 switch(scenario){
  case 0:return 10500000u; /* held bass */
  case 1:return 0x1fffffffu; /* exact band boundary */
  case 2:return (frame&1u)?0x1fffffffu:0x20000000u; /* cross mip */
  case 3:return 0x0fffffffu+(uint32_t)(frame%3u)-1u;
  case 4:return 6200000u+(frame/128u%17u)*23000u; /* held/glide */
  case 5:return 1000000u+(frame%4096u)*172u; /* continuously gliding */
  case 6:return (frame%256u)<128u?5320000u:10700000u;
  case 7:return 0u; /* degenerate pitch guard */
  case 8:return 0x7fffffffu;
  case 9:return 0x80000000u;
  case 10:return 0xffffffffu;
  case 11:return (frame&31u)<16u?4050000u:4080000u;
  case 12:return 0x03ffffffu+(frame%7u)-3u;
  case 13:return (frame*24191u)^0x05ff55ffu;
  case 14:return 6285000u;
  default:return 0x015fffffu+(frame%2048u);
 }
}

int main(void){
 assert(sizeof(RuGrowlMipCache)<=12u);
 uint32_t frames=0u,reused=0u;
 for(uint8_t scenario=0u;scenario<16u;scenario++){
  uint32_t phase=0u;
  RuGrowlState old={0},fast={0};
  RuGrowlMipCache cache={0};
  RuGrowlParams p=ru_growl_make_params(58u,78u,62u,68u,62u,74u,80u,35u);
  for(uint32_t frame=0u;frame<65536u;frame++){
   uint32_t inc=pitch_at(scenario,frame);
   phase+=inc;
   if(!(frame&127u)){
    /* A fresh cache is built each ALNK DMA block in firmware. */
    cache.valid=0u;
    p=ru_growl_make_params((uint8_t)((frame/128u)%101u),78u,62u,68u,
                           62u,74u,80u,35u);
   }
   if(cache.valid&&cache.increment==inc)reused++;
   int32_t a=ru_growl_read(phase,inc);
   int32_t b=ru_growl_read_cached(phase,inc,&cache);
   if(a!=b){
    fprintf(stderr,"V2.0.25 mip PCM difference scenario=%u frame=%u phase=%u inc=%u old=%d cached=%d\n",
      scenario,frame,phase,inc,a,b);
    return 1;
   }
   int32_t y0=ru_growl_process(&old,a,&p);
   int32_t y1=ru_growl_process(&fast,b,&p);
   assert(y0==y1);
   frames++;
  }
 }
 assert(frames==16u*65536u);
 assert(reused>300000u);
 printf("PASS V2.0.25 GROWL mip-cache parity: %u carrier + formant frames, %u reused increment decisions\n",
        frames,reused);
 return 0;
}
