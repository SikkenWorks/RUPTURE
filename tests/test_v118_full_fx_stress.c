/* SPDX-License-Identifier: GPL-3.0-only
 * CPU host PCM stress is NOT a real FM1 DMA deadline certification.
 * Exercises the exact embedded post-mix FX with all current RUPTURE FX
 * simultaneously at 100% and maximum feedback / filter gain.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx_bypass_v181.h"
static struct etch_native_fx fx;
static struct etch_fx_settings cfg;
static void check(int condition,const char*detail){if(!condition){fprintf(stderr,"FX STRESS FAILED: %s\n",detail);exit(2);}}
static void all_max(void){
 etch_fx_defaults(&cfg);
 cfg.comp_threshold=32767;cfg.comp_ratio=32767;cfg.comp_attack=32767;cfg.comp_release=32767;
 cfg.folder_fold=32767;cfg.folder_symmetry=32767;cfg.folder_tone=32767;cfg.folder_mix=32767;
 cfg.chorus_rate=32767;cfg.chorus_depth=32767;
 cfg.delay_time=32767;cfg.delay_feedback=32767;
 cfg.reverb_size=32767;cfg.reverb_decay=32767;
 cfg.eq_low=32767;cfg.eq_mid=32767;cfg.eq_high=32767;
 rupture_transient_controls(100,100,100,100,1);
 rupture_tilt_controls(100,100,100,100,1);
 rupture_fuzz_controls(100,100,100,100,1);
 rupture_sandpaper_controls(100,100,100,100,1);
}
static void neutral(void){
 etch_fx_defaults(&cfg);
 rupture_transient_controls(50,50,60,0,0);
 rupture_tilt_controls(50,50,0,50,0);
 rupture_fuzz_controls(50,75,70,0,0);
 rupture_sandpaper_controls(60,40,45,80,0);
}
int main(void){
 etch_fx_reset(&fx);neutral();
 int16_t l,r;uint64_t drydiff=0;
 for(int i=0;i<2048;i++){
  int16_t input=(int16_t)(i*31%19000-9500);
  etch_fx_process_stereo_masked(&fx,input,0,0,0,&cfg,0u,&l,&r);
  drydiff+=(uint64_t)abs(l-input)+abs(r-input);
 }
 check(drydiff==0,"neutral chain must preserve bit-exact bypass");
 all_max();
 /* Includes continuous full-scale input, low bass + upper harmonics,
  * silence/retrigger, and rapid periodical control toggles. */
 uint32_t peak=0,fullscale=0,dropouts=0;uint64_t nonzero=0,energy=0;
 const int N=220500; /* 5 seconds at 44.1 kHz */
 for(int i=0;i<N;i++){
  if(i==N/2)neutral();
  if(i==N/2+18000)all_max();
  int32_t input;
  switch((i/10000)%5){
   case 0:input=(i&1)?32767:-32768;break;
   case 1:input=(int32_t)(32000*sin(2.0*3.141592653589793*63.0*i/44100.0));break;
   case 2:input=(int32_t)(28000*sin(2.0*3.141592653589793*1379.0*i/44100.0));break;
   case 3:input=0;break;
   default:input=(int32_t)(26000*sin(2.0*3.141592653589793*93.0*i/44100.0)+5000*sin(2.0*3.141592653589793*9701.0*i/44100.0));break;
  }
  if(input>32767)input=32767;
  if(input<-32768)input=-32768;
  etch_fx_process_stereo_masked(&fx,(int16_t)input,32767,32767,32767,&cfg,0,&l,&r);
  uint32_t a=(uint32_t)abs((int)l),b=(uint32_t)abs((int)r);if(a>peak)peak=a;if(b>peak)peak=b;
  if(a>=32767||b>=32767)fullscale++;
  if(input!=0&&(a>0||b>0))nonzero++;
  if((a+b)==0&&input>28000)dropouts++;
  energy+=(uint64_t)a+b;
 }
 check(peak<=28000,"maximum wet output must have >= 1.35 dB headroom");
 check(fullscale==0,"no fullscale clipped wet PCM frames");
 check(nonzero>60000,"FX mix must not mute sustained playing");
 check(dropouts<5000,"FX must not produce systematic silence on high input");
 check(energy>50000000,"FX output retains measurable energy");
 neutral();etch_fx_reset(&fx);
 for(int i=0;i<1024;i++){
  int16_t input=(int16_t)((i%128)-64);
  etch_fx_process_stereo_masked(&fx,input,0,0,0,&cfg,0,&l,&r);
  check(l==input&&r==input,"dry output exact after reset");
 }
 printf("PASS V1.18: all ten FX worst-case, peak=%u/32767, clipped=%u, active=%llu, dropouts=%u, energy=%llu, exact dry\n",peak,fullscale,(unsigned long long)nonzero,dropouts,(unsigned long long)energy);
 return 0;
}
