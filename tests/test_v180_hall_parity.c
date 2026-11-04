/* Deterministic FX PCM equivalence stream: compare SHA256 of old/new builds. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"
static struct etch_native_fx fx;
int main(void){
 struct etch_fx_settings c; etch_fx_defaults(&c);
 int16_t l,r;
 for(int typ=1; typ<2; typ++){
  etch_fx_reset(&fx);
  etch_fx_set_reverb_type(&c,(uint8_t)typ);
  c.reverb_size=27000;c.reverb_decay=31000;
  c.chorus_rate=27300;c.chorus_depth=29000;
  c.delay_time=29300;c.delay_feedback=28000;
  c.ring_mix=24000;c.ring_rate=16000;c.ring_shape=21000;
  c.eq_low=23000;c.eq_mid=24500;c.eq_high=20000;
  c.comp_ratio=28000;c.comp_threshold=29000;
  c.trem_depth=21000;c.trem_mix=27000;c.trem_rate=15000;
  c.phaser_depth=25000;c.phaser_mix=27000;c.phaser_rate=18000;
  c.flanger_depth=21000;c.flanger_mix=27000;c.flanger_rate=17000;
  c.decim_bits=9000;c.decim_rate=23000;c.decim_mix=12000;
  c.folder_fold=22000;c.folder_mix=17000;
  c.tape_wow=25000;c.tape_flutter=20000;c.tape_age=18000;c.tape_mix=18000;
  c.reverse_time=7000;c.reverse_feed=17000;c.reverse_mix=18000;
  c.vibrato_rate=28000;c.vibrato_depth=9000;c.vibrato_mix=7000;
  c.rotary_rate=23000;c.rotary_depth=25000;c.rotary_mix=26000;
  c.wah_sens=22000;c.wah_attack=19000;c.wah_reso=10000;c.wah_mix=9000;
  for(uint32_t i=0;i<65000;i++){
   if((i%7001u)==0u){c.reverb_decay=(uint16_t)(16000u+((i*137u)%16768u));c.delay_time=(uint16_t)((i*197u)%32768u);}
   int16_t x=(int16_t)(((i*311u+((i>>3u)*117u))%36000u)-18000);
   etch_fx_process_stereo_masked(&fx,x,22000,27000,(uint16_t)((i/911u)%2u?32767u:15000u),&c,0,&l,&r);
   if(fwrite(&l,sizeof(l),1,stdout)!=1 || fwrite(&r,sizeof(r),1,stdout)!=1)return 2;
  }
 }
 return 0;
}
