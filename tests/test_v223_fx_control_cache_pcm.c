#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"

static struct etch_native_fx reference_fx,cache_fx;

/* Compare the old per-sample coefficient checks to the control-segment
 * cache. Sweep hidden modulation, changed knobs, disabled slots, cold
 * startup and a DSP reset between two 32-sample boundaries. */
int main(void){
 uint32_t frames=0;
 for(uint8_t scenario=0u;scenario<60u;scenario++){
  struct etch_fx_settings fx;etch_fx_defaults(&fx);
  struct rupture_fx_chain chain;ru_chain_clear(&chain);
  chain.independent=(uint8_t)(scenario&1u);
  uint8_t slot=(uint8_t)(scenario%8u);
  uint8_t type=(uint8_t)(scenario%10u);
  assert(ru_chain_insert(&chain,slot,type));
  if((scenario%3u)==0u)assert(ru_chain_insert(&chain,7u,(uint8_t)((type+5u)%10u)));
  if((scenario%5u)==0u){
   assert(ru_chain_insert(&chain,0u,RU_FX_COMP));
   assert(ru_chain_insert(&chain,1u,RU_FX_TILT));
   assert(ru_chain_insert(&chain,2u,RU_FX_FUZZ));
   assert(ru_chain_insert(&chain,3u,RU_FX_SAND));
  }
  fx.folder_mix=27000;fx.folder_fold=18000;
  fx.wah_mix=21000;fx.wah_sens=19000;
  fx.comp_ratio=16000;
  /* The compatibility renderer's hidden path may touch applied caches.
   * Cached playback must explicitly fall back to the old checks here. */
  if((scenario%7u)==0u){fx.trem_mix=20000;fx.trem_depth=19000;}
  etch_fx_reset(&reference_fx);etch_fx_reset(&cache_fx);
  uint8_t checked=0u;
  struct rupture_fx_dispatch dispatch={0};
  for(uint32_t i=0;i<6144u;i++){
   if(!(i&127u)){
    if(i==2048u)chain.enabled=0u;
    if(i==3072u){
     for(uint8_t j=0u;j<RU_FX_SLOTS;j++)
      if(chain.type[j]!=RU_FX_EMPTY)chain.enabled|=(uint8_t)(1u<<j);
    }
    if(i==5120u&&slot!=7u)ru_chain_remove(&chain,slot);
    dispatch=ru_chain_prepare_dispatch(&chain);
   }
   if(!(i&31u)){
    checked=0u;
    /* A realistic C-FX/knob update changes coefficients on this boundary,
     * never the 31 frames between snapshots. */
    fx.chorus_rate=(uint16_t)(2000u+(i*13u+scenario*143u)%30000u);
    fx.delay_time=(uint16_t)(1000u+(i*7u+scenario*51u)%28000u);
    fx.reverb_size=(uint16_t)(4000u+(i*3u+scenario*101u)%26000u);
    fx.comp_attack=(uint16_t)(900u+(i*11u)%25000u);
    fx.tape_wow=(uint16_t)(8000u+(i*23u)%24000u);
   }
   /* Reset inside a control segment to exercise applied_valid fallback. */
   if(i==1427u){etch_fx_reset(&reference_fx);etch_fx_reset(&cache_fx);}
   int16_t sample=(int16_t)((i%73u)<13u?27000:(i&1u)?-19500:15600);
   int16_t a,b,c,d;
   rupture_fx_process_ordered_packed(&reference_fx,sample,24000,20000,26000,18000,
      &fx,&chain,0u,&a,&b,&dispatch);
   rupture_fx_process_ordered_cached(&cache_fx,sample,24000,20000,26000,18000,
      &fx,&chain,0u,&c,&d,&dispatch,&checked);
   if(a!=c||b!=d){
    fprintf(stderr,"V2.0.23 FX cache PCM mismatch scenario=%u frame=%u (%d/%d vs %d/%d)\n",
      scenario,i,a,b,c,d);
    return 1;
   }
   frames++;
  }
 }
 assert(frames==60u*6144u);
 printf("PASS V2.0.23 FX control cache: %u stereo frames sample-identical\n",frames);
 return 0;
}
