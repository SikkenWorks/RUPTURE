/* V1.86.1: K3 curve changes actual sustained oscillator audio before FX. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/audio_core.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_bend_curves.h"
static struct etch_native_audio osc[RU_BEND_CURVE_COUNT];
static struct etch_native_controls controls[RU_BEND_CURVE_COUNT];
static int16_t pcm[RU_BEND_CURVE_COUNT][256];
static void run(uint8_t saw){
 uint8_t raw=35u;
 uint32_t factor[RU_BEND_CURVE_COUNT];
 for(uint8_t i=0;i<RU_BEND_CURVE_COUNT;i++){
  uint32_t shaped=ru_bend_curve_percent(i,raw);
  factor[i]=65536u+((73562u-65536u)*shaped+50u)/100u;
  memset(&controls[i],0,sizeof(controls[i]));
  etch_fx_defaults(&controls[i].fx_settings);
  controls[i].mono_mode=1u;controls[i].main_wave=saw;
  controls[i].gain=19000u;controls[i].cutoff=17000u;
  controls[i].sustain_level=25000u;
  controls[i].attack_step=0x40000000u;
  controls[i].decay_step=0x00008800u;
  controls[i].release_step=0x00008800u;
  controls[i].cxs_pressure=74u;controls[i].cxs_balance=50u;
  controls[i].cxs_memory=55u;controls[i].cxs_density=63u;
  controls[i].cxs_weight=38u;controls[i].cxs_edge=46u;
  controls[i].cxs_bite=26u;controls[i].cxs_punch=40u;
  controls[i].live_bend_q16=factor[i];
  etch_native_reset(&osc[i]);
  etch_native_midi(&osc[i],0x90,48,112);
 }
 for(uint8_t i=0;i<RU_BEND_CURVE_COUNT;i++)
  for(uint8_t j=i+1;j<RU_BEND_CURVE_COUNT;j++)assert(factor[i]!=factor[j]);
 for(int block=0;block<30;block++)
  for(uint8_t i=0;i<RU_BEND_CURVE_COUNT;i++){
   etch_native_render_stereo(&osc[i],&controls[i],0u,pcm[i],128u);
   assert(osc[i].mono_count==1u);
  }
 for(uint8_t i=0;i<RU_BEND_CURVE_COUNT;i++)
  for(uint8_t j=i+1;j<RU_BEND_CURVE_COUNT;j++)
   assert(memcmp(pcm[i],pcm[j],sizeof(pcm[i]))!=0);
 /* Stopping encoder = 0%: exactly the old neutral multiplier, with no Note OFF */
 for(uint8_t i=0;i<RU_BEND_CURVE_COUNT;i++){
  assert(ru_bend_curve_percent(i,0u)==0u);
  controls[i].live_bend_q16=65536u;
  for(int n=0;n<8;n++)etch_native_render_stereo(&osc[i],&controls[i],0u,pcm[i],128u);
  assert(osc[i].mono_count==1u);
 }
}
int main(void){run(0u);run(1u);puts("PASS: 5 K3 response profiles change actual CXS+SAW sustained PCM, no retrigger, returns neutral");return 0;}
