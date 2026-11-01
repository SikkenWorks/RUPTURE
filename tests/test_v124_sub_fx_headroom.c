/* V1.24: combined SUB + all active FX peak and selector UI regression.
 * This is HOST PCM validation, not a physical FM1 DMA deadline proof. */
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_subosc.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"
static struct etch_native_fx fx;
static struct etch_fx_settings c;
#define N 110250
int main(void){
 RuSubState a,b;ru_sub_init(&a,5300000,777);ru_sub_init(&b,5300000,555);
 etch_fx_reset(&fx);etch_fx_defaults(&c);
 c.comp_threshold=32767;c.comp_ratio=32767;c.comp_attack=32767;c.comp_release=32767;
 c.eq_low=32767;c.eq_mid=32767;c.eq_high=32767;c.eq_hpf=32767;
 c.delay_time=32767;c.delay_feedback=32767;c.chorus_depth=32767;c.chorus_rate=32767;
 c.reverb_size=32767;c.reverb_decay=32767;
 rupture_tilt_controls(100,100,100,100,1);
 rupture_fuzz_controls(100,100,100,100,1);
 rupture_sandpaper_controls(100,100,100,100,1);
 int max=0,clips=0,active=0;
 double classic_energy=0, main_energy=0;
 int last=0,maxjump=0;
 for(int i=0;i<N;i++){
  uint8_t blend=(i/8000u)&1u?100u:0u; /* sudden ALGORITHM changes */
  uint8_t type_a=i<N/2?2u:1u,type_b=i<N/2?1u:2u;
  int32_t av=ru_sub_tick(&a,type_a,1u,50u,50u,5300000,78,88,83,91);
  int32_t bv=ru_sub_tick(&b,type_b,1u,55u,70u,5300000,78,88,83,91);
  uint16_t ag=ru_sub_smooth_blend(&a,blend);
  uint16_t bg=ru_sub_smooth_blend(&b,100u);
  int32_t main=(int32_t)(22000.0*sin((6.283185307179586*84.0*i)/44100.0));
  int32_t cv=ru_sub_blend(main,type_a==2u?0:av,type_b==2u?0:bv,ag,bg);
  /* Simulate lowpass: main and CXS sub attenuated; CLASSIC post filter. */
  cv=(cv*3)/4;
  cv=ru_sub_blend(cv,type_a==2u?av:0,type_b==2u?bv:0,ag,bg);
  if(i>10000&&type_a==2u&&ag>0)classic_energy+=(double)av*av;
  main_energy+=(double)main*main;
  assert(cv<=28300&&cv>=-28300);
  int16_t l=0,r=0;
  etch_fx_process_stereo_masked(&fx,(int16_t)cv,32767,32767,32767,&c,0,&l,&r);
  int ma=abs((int)l);if(abs((int)r)>ma)ma=abs((int)r);
  if(ma>max)max=ma;
  if(ma>=32767)clips++;
  if(ma>100)active++;
  int jump=abs((int)l-last);if(jump>maxjump)maxjump=jump;
  last=l;
 }
 assert(classic_energy>100000000000.0);
 assert(main_energy>100000000000.0);
 assert(clips==0&&max<=28000&&active>N/6);
 /* Below maximum PCM this only proves no fullscale jumps, not no clicks in hardware. */
 printf("PASS V1.24: CXS/CLASSIC dual SUB+FX stress, max=%d, fullscale=%d, active=%d, max_delta=%d\n",max,clips,active,maxjump);
 /* Full blend transition is bounded and never an instant ADC gain change. */
 RuSubState ramp;ru_sub_init(&ramp,5300000,999);ramp.mix=RU_SUB_MIX_MAX;
 assert(ru_sub_smooth_blend(&ramp,100)==64);
 for(int i=0;i<400;i++)(void)ru_sub_smooth_blend(&ramp,100);
 assert(ramp.blend_smooth==RU_SUB_MIX_MAX);
 assert(ru_sub_smooth_blend(&ramp,0)==RU_SUB_MIX_MAX-64);
 return 0;
}
