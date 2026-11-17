/* Golden left/right PCM for older PJT35 FX routing and hidden mod FX.
 * Realtime cache bypass is only safe when every consumer is inactive. */
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"

static struct etch_native_fx fx;
static uint32_t render(uint8_t independent){
 struct etch_fx_settings c;
 struct rupture_fx_chain chain;
 etch_fx_defaults(&c);ru_chain_clear(&chain);
 assert(ru_chain_insert(&chain,0u,RU_FX_WAH));
 assert(ru_chain_insert(&chain,1u,RU_FX_REVERB));
 assert(ru_chain_insert(&chain,2u,RU_FX_CHORUS));
 assert(ru_chain_insert(&chain,3u,RU_FX_DELAY));
 chain.independent=independent;
 c.wah_mix=20000u;c.wah_sens=16000u;
 c.reverb_size=14000u;c.chorus_depth=12000u;c.delay_time=9000u;
 c.trem_rate=8000u;c.chorus_rate=12000u;
 etch_fx_reset(&fx);
 uint32_t hash=2166136261u;
 for(uint32_t n=0u;n<36000u;n++){
  if(n==7500u){c.trem_mix=18000u;c.vibrato_mix=9000u;}
  if(n==14000u){c.chorus_rate=30000u;c.delay_time=25000u;}
  if(n==19500u){c.trem_mix=0u;c.vibrato_mix=0u;}
  if(n==25000u){c.chorus_rate=11000u;c.reverb_size=29000u;}
  if(n==28000u){c.flanger_mix=12000u;c.flanger_rate=23000u;}
  if(n==32000u){c.flanger_mix=0u;}
  int16_t input=(int16_t)((n*1013u+(n>>5u)*431u)&65535u);
  int16_t left=0,right=0;
  rupture_fx_process_ordered(&fx,input,22000u,18000u,27000u,19000u,
    &c,&chain,0u,&left,&right);
  hash=(hash^(uint16_t)left)*16777619u;
  hash=(hash^(uint16_t)right)*16777619u;
 }
 return hash;
}

int main(void){
 uint32_t independent=render(1u),legacy=render(0u);
#ifdef RU_V204_BASELINE
 if(independent!=2822464508u||legacy!=1687728378u){
  fprintf(stderr,"FAIL historical V1.94 legacy/hidden PCM oracle: %u %u\n",independent,legacy);
  return 1;
 }
 puts("PASS original V1.94 hidden modulation and legacy FX PCM baseline");
#else
 /* ROOM/HALL's upgraded diffusion intentionally changes these two outputs.
  * These hashes are the exact V2.0.4 result, not the old V1.94 oracle. */
 if(independent!=1088817010u||legacy!=2620323848u){
  fprintf(stderr,"FAIL V2.0.4 legacy/hidden PCM oracle: %u %u\n",independent,legacy);
  return 1;
 }
 puts("PASS V2.0.4 hidden modulation and legacy FX deterministic PCM");
#endif
 return 0;
}
