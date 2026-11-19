#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"

static struct etch_native_fx reference_fx,fast_fx;

/* Original per-sample DISTORTION coefficient math versus the new control-
 * segment cache. Exercise GROWL-like high-drive presets, repeated inserts,
 * independent banks, bypass ramps, dynamic knob edits and resets. */
int main(void){
 assert(sizeof(struct rupture_fx_distortion_cache)<=52u);
 uint32_t checked_frames=0u,cache_hits=0u;
 for(uint8_t scenario=0u;scenario<64u;scenario++){
  struct etch_fx_settings fx;etch_fx_defaults(&fx);
  struct rupture_fx_chain chain;ru_chain_clear(&chain);
  chain.independent=(uint8_t)(scenario&1u);
  uint8_t primary_slot=(uint8_t)(scenario%8u);
  assert(ru_chain_insert(&chain,primary_slot,RU_FX_DISTORTION));
  if(scenario%3u==0u)assert(ru_chain_insert(&chain,7u,RU_FX_DISTORTION));
  if(scenario%5u==0u)assert(ru_chain_insert(&chain,0u,RU_FX_WAH));
  if(scenario%7u==0u)assert(ru_chain_insert(&chain,5u,RU_FX_DISTORTION));
  fx.folder_mix=31000u;
  fx.folder_fold=30000u;
  fx.folder_symmetry=26000u;
  fx.folder_tone=12000u;
  fx.wah_mix=11000u;
  if(scenario%9u==0u)fx.trem_mix=17000u; /* legacy hidden path */
  for(uint8_t slot=0u;slot<RU_FX_SLOTS;slot++)if(chain.type[slot]==RU_FX_DISTORTION){
   chain.parameter[slot][0]=(uint16_t)(12000u+slot*1300u);
   chain.parameter[slot][1]=(uint16_t)(10000u+slot*900u);
   chain.parameter[slot][2]=(uint16_t)(28000u-slot*1700u);
   chain.parameter[slot][3]=32000u;
  }
  etch_fx_reset(&reference_fx);etch_fx_reset(&fast_fx);
  struct rupture_fx_dispatch dispatch={0};
  uint8_t coefficient_checked=0u;
  uint8_t fast_coefficient_checked=0u;
  struct rupture_fx_distortion_cache distortion_cache={0};
  for(uint32_t frame=0u;frame<8192u;frame++){
   if(!(frame&127u)){
    if(frame==2048u)chain.enabled=0u;
    if(frame==2560u){
     for(uint8_t slot=0u;slot<RU_FX_SLOTS;slot++)
      if(chain.type[slot]!=RU_FX_EMPTY)chain.enabled|=(uint8_t)(1u<<slot);
    }
    if(frame==4608u&&chain.independent){
     for(uint8_t slot=0u;slot<RU_FX_SLOTS;slot++)
      if(chain.type[slot]==RU_FX_DISTORTION)
       chain.parameter[slot][0]=(uint16_t)(24000u+(slot*800u));
    }
    dispatch=ru_chain_prepare_dispatch(&chain);
   }
   if(!(frame&31u)){
    coefficient_checked=0u;
    distortion_cache.valid_mask=0u;
    /* Wave C FX edits arrive at the same control-rate boundary. */
    fx.folder_fold=(uint16_t)((frame*13u+scenario*37u)%32768u);
    fx.folder_tone=(uint16_t)((frame*17u+scenario*103u)%32768u);
    fx.folder_symmetry=(uint16_t)((frame*7u+scenario*221u)%32768u);
   }
   if(frame==1351u){etch_fx_reset(&reference_fx);etch_fx_reset(&fast_fx);}
   const int16_t sample=(int16_t)((frame%227u)<32u?30000:(frame&1u)?-28000:18000);
   int16_t l0,r0,l1,r1;
   rupture_fx_process_ordered_cached(&reference_fx,sample,25000u,18000u,26000u,18000u,
    &fx,&chain,0u,&l0,&r0,&dispatch,&coefficient_checked);
   /* Reuse the coefficient validity flag only in the candidate. The
    * original applied-cache path keeps a separate state in each renderer. */
   if(!(frame&31u))fast_coefficient_checked=0u;
   rupture_fx_process_ordered_fast(&fast_fx,sample,25000u,18000u,26000u,18000u,
    &fx,&chain,0u,&l1,&r1,&dispatch,&fast_coefficient_checked,&distortion_cache);
   if(l0!=l1||r0!=r1){
    fprintf(stderr,"V2.0.24 DISTORTION PCM mismatch case=%u frame=%u old=%d/%d fast=%d/%d\n",
      scenario,frame,l0,r0,l1,r1);
    return 1;
   }
   if(distortion_cache.valid_mask)cache_hits++;
   checked_frames++;
  }
 }
 assert(checked_frames==64u*8192u);
 assert(cache_hits>200000u);
 printf("PASS V2.0.24 DISTORTION coefficient cache: %u stereo frames sample-identical\n",checked_frames);
 return 0;
}
