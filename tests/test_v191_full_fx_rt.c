#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_fx_chain.h"

static struct etch_native_fx fx;
int main(void){
 struct rupture_fx_chain chain;ru_chain_clear(&chain);
 static const uint8_t order[8]={RU_FX_COMP,RU_FX_TILT,RU_FX_FUZZ,RU_FX_SAND,
  RU_FX_CHORUS,RU_FX_DELAY,RU_FX_REVERB,RU_FX_MOSFET};
 for(unsigned n=0;n<8;n++)assert(ru_chain_insert(&chain,(uint8_t)n,order[n]));
 struct etch_fx_settings settings;etch_fx_defaults(&settings);
 settings.comp_ratio=32000;settings.comp_threshold=28000;
 settings.reverb_decay=32767;settings.reverb_size=31000;
 settings.delay_time=32767;settings.delay_feedback=32000;
 settings.chorus_depth=32000;settings.chorus_rate=30000;
 etch_fx_reset(&fx);
 rupture_tilt_controls(90,95,100,65,1);
 rupture_fuzz_controls(80,100,90,100,1);
 rupture_sandpaper_controls(90,90,85,100,1);
 int changed=0,silent=0;uint32_t digest=0;
 for(unsigned i=0;i<180000u;i++){
  /* Exercise all 8 effects, then many order changes and ON/OFF transitions
   * with full-scale pulses. Never allocate/clear an FX tank in the IRQ. */
  if(i>0u && i%5000u==0u){
   uint8_t where=(uint8_t)((i/5000u)%7u);
   assert(ru_chain_move(&chain,where,1));changed++;
  }
  if(i==100000u){
   chain.enabled=0u;
   rupture_tilt_controls(90,95,100,65,0);
   rupture_fuzz_controls(80,100,90,100,0);
   rupture_sandpaper_controls(90,90,85,100,0);
  }
  int16_t l=0,r=0;
  int16_t sample=(i%97u==0u)?30000:(i%13u<6u?18000:-18000);
  rupture_fx_process_ordered(&fx,sample,32000,32000,32000,30000,
   &settings,&chain,0,&l,&r);
  digest+=(uint16_t)l+(uint16_t)r;
  if(i>140000u && l==sample && r==sample)silent++;
 }
 assert(changed>=20);
 assert(silent>35000); /* dry path settles after bounded tail drain */
 assert(digest!=0u);
 puts("PASS V1.91 8 simultaneous FX, live reorder, fast dry drain, bounded PCM");
 return 0;
}
