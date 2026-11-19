#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"

static struct etch_native_fx reference_fx,compact_fx;

int main(void){
 const uint8_t types[]={RU_FX_COMP,RU_FX_TILT,RU_FX_FUZZ,RU_FX_SAND,
  RU_FX_CHORUS,RU_FX_DELAY,RU_FX_REVERB,RU_FX_MOSFET,
  RU_FX_DISTORTION,RU_FX_WAH};
 struct etch_fx_settings c;etch_fx_defaults(&c);
 c.comp_threshold=20000u;c.comp_ratio=21000u;
 c.delay_time=23000u;c.delay_feedback=18000u;
 c.reverb_size=27000u;c.reverb_decay=19000u;
 c.folder_fold=24500u;c.folder_mix=28000u;
 c.wah_mix=25000u;c.wah_sens=23000u;
 rupture_tilt_controls(75u,55u,65u,45u,1u);
 rupture_fuzz_controls(65u,70u,60u,85u,1u);
 rupture_sandpaper_controls(70u,50u,40u,75u,1u);
 uint32_t frames=0u;
 for(uint8_t scenario=0u;scenario<24u;scenario++){
  struct rupture_fx_chain chain;ru_chain_clear(&chain);
  chain.independent=(uint8_t)(scenario&1u);
  uint8_t end=(uint8_t)(scenario%9u);
  for(uint8_t k=0;k<end;k++){
   uint8_t ix=(uint8_t)((scenario+k*3u)%10u);
   if((scenario%4u)!=3u || k==0u || k==end-1u)
    assert(ru_chain_insert(&chain,k,types[ix]));
  }
  if(end&&scenario%5u==0u)chain.enabled=0u;
  assert(ru_chain_span(&chain)==(end?end:0u));
  etch_fx_reset(&reference_fx);etch_fx_reset(&compact_fx);
  uint8_t span=ru_chain_span(&chain);
  for(uint32_t i=0u;i<12288u;i++){
   if((i&127u)==0u){
    /* A knob/page change arrives only at the same control publication
     * boundary at which the real audio ISR recomputes its compact span. */
    if(i==4096u&&end){chain.enabled^=(uint8_t)(1u<<(end-1u));}
    if(i==8192u&&end){chain.type[end-1u]=RU_FX_EMPTY;chain.enabled&=(uint8_t)~(1u<<(end-1u));}
    span=ru_chain_span(&chain);
   }
   int16_t input=(int16_t)((i%233u)<29u?25000:(i&1u)?-17000:18000);
   int16_t l0,r0,l1,r1;
   rupture_fx_process_ordered(&reference_fx,input,22000u,25000u,27000u,16000u,
     &c,&chain,0u,&l0,&r0);
   rupture_fx_process_ordered_compact(&compact_fx,input,22000u,25000u,27000u,16000u,
     &c,&chain,0u,&l1,&r1,span);
   if(l0!=l1||r0!=r1){
    fprintf(stderr,"V2.0.21 PCM mismatch: case=%u frame=%u span=%u ref=%d/%d opt=%d/%d\n",
      scenario,i,span,l0,r0,l1,r1);
    return 1;
   }
   frames++;
  }
 }
 assert(ru_chain_span(NULL)==0u);
 assert(frames==24u*12288u);
 printf("PASS V2.0.21 FX trailing-empty spans: %u stereo frames sample-exact\n",frames);
 return 0;
}
