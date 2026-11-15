/* No sound-quality change on an existing SINGLE FX instance when its exact
 * parameters are migrated from V1.93 to a V1.94 independent slot. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"

static struct etch_native_fx old_fx,new_fx;
int main(void){
 struct etch_fx_settings c;etch_fx_defaults(&c);
 c.comp_threshold=15000u;c.comp_ratio=19500u;c.comp_attack=8000u;c.comp_release=12000u;
 c.folder_fold=16000u;c.folder_symmetry=15000u;c.folder_tone=22000u;c.folder_mix=24000u;
 c.wah_sens=24000u;c.wah_attack=10000u;c.wah_reso=22000u;c.wah_mix=21000u;
 rupture_tilt_controls(70u,35u,80u,60u,1u);
 rupture_fuzz_controls(55u,75u,65u,65u,1u);
 rupture_sandpaper_controls(60u,40u,45u,80u,1u);
 for(uint8_t type=0u;type<RU_FX_TYPES;type++){
  struct rupture_fx_chain legacy,modern;
  ru_chain_clear(&legacy);ru_chain_clear(&modern);
  assert(ru_chain_insert(&legacy,0u,type));
  assert(ru_chain_insert(&modern,0u,type));
  legacy.independent=0u;
  uint16_t *p=modern.parameter[0];
  switch(type){
   case RU_FX_REVERB:p[0]=etch_fx_reverb_type(&c);p[1]=c.reverb_decay;
    p[2]=c.reverb_size;p[3]=20000u;break;
   case RU_FX_DELAY:p[0]=c.delay_time;p[1]=c.delay_feedback;
    p[2]=etch_fx_route(&c);p[3]=19000u;break;
   case RU_FX_CHORUS:p[0]=c.chorus_rate;p[1]=c.chorus_depth;
    p[2]=etch_fx_route(&c);p[3]=18000u;break;
   case RU_FX_MOSFET:p[0]=14000u;break;
   case RU_FX_COMP:p[0]=c.comp_threshold;p[1]=c.comp_ratio;
    p[2]=c.comp_attack;p[3]=c.comp_release;break;
   case RU_FX_TILT:p[0]=70u;p[1]=35u;p[2]=80u;p[3]=60u;break;
   case RU_FX_FUZZ:p[0]=55u;p[1]=75u;p[2]=65u;p[3]=65u;break;
   case RU_FX_SAND:p[0]=60u;p[1]=40u;p[2]=45u;p[3]=80u;break;
   case RU_FX_DISTORTION:p[0]=c.folder_fold;p[1]=c.folder_symmetry;
    p[2]=c.folder_tone;p[3]=c.folder_mix;break;
   case RU_FX_WAH:p[0]=c.wah_sens;p[1]=c.wah_attack;p[2]=c.wah_reso;p[3]=c.wah_mix;break;
  }
  etch_fx_reset(&old_fx);etch_fx_reset(&new_fx);
  for(uint32_t n=0u;n<12000u;n++){
   int16_t input=(n%97u)<29u?19000:-16000;
   int16_t lo=0,ro=0,ln=0,rn=0;
   rupture_fx_process_ordered(&old_fx,input,18000u,19000u,20000u,14000u,
    &c,&legacy,0u,&lo,&ro);
   rupture_fx_process_ordered(&new_fx,input,18000u,19000u,20000u,14000u,
    &c,&modern,0u,&ln,&rn);
   if(lo!=ln||ro!=rn){
    fprintf(stderr,"PCM parity mismatch type %u frame %u (%d,%d) != (%d,%d)\n",
      type,n,lo,ro,ln,rn);
    return 1;
   }
  }
 }
 puts("PASS V1.94 sample-exact old/new PCM across all 10 single-effect types");
 return 0;
}
