#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"

static struct etch_native_fx fx;

static uint32_t play(uint8_t mode){
 struct etch_fx_settings c;etch_fx_defaults(&c);
 struct rupture_fx_chain chain;ru_chain_clear(&chain);
 chain.independent=0u; /* Preserve V1.92 exact PCM reference path */
 etch_fx_reset(&fx);
 c.folder_fold=27000u;c.folder_symmetry=27000u;
 c.folder_tone=24000u;c.folder_mix=27000u;
 c.wah_sens=30000u;c.wah_attack=25000u;
 c.wah_reso=32000u;c.wah_mix=26000u;
 if(mode==1u)assert(ru_chain_insert(&chain,0u,RU_FX_DISTORTION));
 if(mode==2u)assert(ru_chain_insert(&chain,0u,RU_FX_WAH));
 if(mode==3u||mode==4u){
  const uint8_t order[8]={RU_FX_DISTORTION,RU_FX_WAH,RU_FX_REVERB,
   RU_FX_MOSFET,RU_FX_CHORUS,RU_FX_COMP,RU_FX_TILT,RU_FX_DELAY};
  for(uint8_t i=0;i<8u;i++)assert(ru_chain_insert(&chain,i,order[i]));
  if(mode==4u)assert(ru_chain_move(&chain,0u,1));
 }
 uint32_t digest=0u;uint32_t distinct=0u;
 for(uint32_t i=0;i<76000u;i++){
  int16_t in=(i%123u<60u)?28600:-29100;
  int16_t l=0,r=0;
  rupture_fx_process_ordered(&fx,in,16500u,15500u,18500u,24000u,
   &c,&chain,0u,&l,&r);
  digest=(digest*33u)^(uint16_t)l^(uint32_t)((uint16_t)r<<1u);
  if(i>3200u && (l!=in||r!=in))distinct++;
 }
 assert(mode==0u?distinct==0u:distinct>5000u);
 return digest;
}
static void no_premature_wah(void){
 struct rupture_fx_chain chain;ru_chain_clear(&chain);
 chain.independent=0u; /* Historical legacy globals */
 assert(ru_chain_insert(&chain,0u,RU_FX_COMP));
 assert(ru_chain_insert(&chain,1u,RU_FX_TILT));
 assert(ru_chain_insert(&chain,2u,RU_FX_FUZZ));
 assert(ru_chain_insert(&chain,3u,RU_FX_SAND));
 struct etch_fx_settings c;etch_fx_defaults(&c);
 c.comp_ratio=20000u;c.wah_sens=30000u;c.wah_mix=32767u;
 c.folder_mix=32767u;c.folder_fold=26000u;
 etch_fx_reset(&fx);
 rupture_tilt_controls(50u,65u,70u,60u,1u);
 rupture_fuzz_controls(20u,30u,40u,50u,1u);
 rupture_sandpaper_controls(60u,50u,35u,55u,1u);
 for(uint32_t i=0u;i<22000u;i++){
  int16_t l=0,r=0;
  rupture_fx_process_ordered(&fx,16000,0,0,0,0,&c,&chain,0,&l,&r);
 }
 /* The old combined four-FX fastpath must never invoke a later/absent
  * wah or distortion, regardless of their stored nonzero wet settings. */
 assert(fx.wah_env==0 && fx.wah_lp==0 && fx.wah_hp==0);
 assert(fx.wah_mix_smooth==0 && fx.folder_mix_smooth==0);
}
int main(void){
 struct rupture_fx_chain chain;ru_chain_clear(&chain);
 assert(ru_chain_insert(&chain,0u,RU_FX_DISTORTION));
 assert(ru_chain_insert(&chain,1u,RU_FX_WAH));
 assert(ru_chain_insert(&chain,2u,RU_FX_DISTORTION));
 assert(!ru_chain_insert(&chain,2u,RU_FX_TYPES));
 assert(ru_chain_valid(&chain));
 assert(ru_chain_move(&chain,1u,1));
 assert(chain.type[2]==RU_FX_WAH && (chain.enabled&4u));
 uint32_t dry=play(0u),dist=play(1u),wah=play(2u);
 uint32_t both=play(3u),reordered=play(4u);
 assert(dist!=dry && wah!=dry && dist!=wah);
 assert(both!=reordered);
 /* V1.93 must preserve original single-instance V1.92 PCM bit for bit. */
 assert(both==3479413712u && reordered==4007301924u);
 no_premature_wah();
 printf("PASS V1.92: 10 available FX, 8 slots, real distortion/wah PCM, routing %u/%u\n",both,reordered);
 return 0;
}
