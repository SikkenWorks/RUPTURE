#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_v190_project.h"

static int32_t play(const struct rupture_fx_chain *chain){
 struct etch_native_fx fx;
 struct etch_fx_settings cfg;
 etch_fx_defaults(&cfg);etch_fx_reset(&fx);
 cfg.comp_threshold=27000u;cfg.comp_ratio=32767u;
 int32_t result=0;
 for(uint32_t n=0;n<1400u;n++){
  int16_t l=0,r=0;
  int16_t input=(n%79u)<32u?28000:-25000;
  rupture_fx_process_ordered(&fx,input,0,0,0,28000,&cfg,chain,0,&l,&r);
  result+=(int32_t)l*(int32_t)((n%17u)+1u);
 }
 return result;
}
static void stereo_after_drive(void){
 struct rupture_fx_chain c;ru_chain_clear(&c);
 assert(ru_chain_insert(&c,0,RU_FX_DELAY));
 assert(ru_chain_insert(&c,1,RU_FX_MOSFET));
 struct etch_native_fx fx;struct etch_fx_settings cfg;
 etch_fx_defaults(&cfg);etch_fx_reset(&fx);
 cfg.delay_time=0u; /* shortest known tap: 384 samples */
 int decorrelated=0;
 for(unsigned n=0;n<1500u;n++){
  int16_t l=0,r=0;
  int16_t input=(n==0u||n==99u)?25000:0;
  rupture_fx_process_ordered(&fx,input,0,28000,0,24000,&cfg,&c,0,&l,&r);
  if(l!=r)decorrelated=1;
 }
 assert(decorrelated); /* post-FX drive must not collapse delay stereo */
}
int main(void){
 struct rupture_fx_chain c;ru_chain_clear(&c);
 assert(ru_chain_valid(&c));
 for(int i=0;i<8;i++)assert(c.type[i]==RU_FX_EMPTY);
 assert(ru_chain_insert(&c,0u,RU_FX_COMP));
 assert(ru_chain_insert(&c,3u,RU_FX_COMP)); /* V1.93: repeated types are allowed */
 assert(ru_chain_valid(&c));
 ru_chain_remove(&c,3u);
 assert(ru_chain_insert(&c,1u,RU_FX_MOSFET));
 assert(c.enabled==3u);
 int32_t original=play(&c);
 assert(ru_chain_move(&c,0u,1));
 assert(c.type[0]==RU_FX_MOSFET && c.type[1]==RU_FX_COMP);
 assert(c.enabled==3u);
 int32_t reordered=play(&c);
 assert(original!=reordered); /* permutation changes real output */
 ru_chain_toggle(&c,0u);
 assert(c.enabled==2u);
 assert(ru_chain_move(&c,0u,1));
 assert(c.enabled==1u); /* bypass follows the moved effect */
 ru_chain_remove(&c,1u);
 assert(c.type[1]==RU_FX_EMPTY && ru_chain_valid(&c));
 struct rupture_v190_project p;ru_v190_defaults(&p);
 assert(ru_v190_valid(&p));assert(!ru_slide_at(&p,17u));
 ru_slide_set(&p,17u,1u);assert(ru_slide_at(&p,17u));
 ru_slide_set(&p,17u,0u);assert(!ru_slide_at(&p,17u));
 stereo_after_drive();
 puts("PASS V1.90 eight slots / real PCM FX ordering / stereo drive / slide bits");
 return 0;
}
