/* Offline DSP tests: actual native SAW and actual post-mix reverb implementations. */
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_bandwaves.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_saw_tone.h"
#include "rupture_saw_tone_v176.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"
static struct etch_native_fx fx;
static void saw_test(void){
 uint32_t phase=0u,inc=(uint32_t)((double)0x100000000ULL*65./44100.);
 int32_t oldlo=0,newlo=0;double old_hf=0,new_hf=0,old_rms=0,new_rms=0;
 int32_t lastold=0,lastnew=0;int oldclip=0,newclip=0;
 for(int i=0;i<44100;i++){
  phase+=inc;
  int32_t raw=ru_bandwave_read(RU_WAVE_SAW,phase,inc);
  int32_t a=rupture_saw_tone_v176(&oldlo,raw,58,50,52,68,50,0,0,0);
  int32_t b=rupture_saw_tone(&newlo,raw,58,50,52,68,50,0,0,0);
  if(i>1000){old_hf+=(double)(a-lastold)*(a-lastold);new_hf+=(double)(b-lastnew)*(b-lastnew);old_rms+=(double)a*a;new_rms+=(double)b*b;}
  if(a>=32767||a<=-32767)oldclip++;
  if(b>=32767||b<=-32767)newclip++;
  lastold=a;lastnew=b;
 }
 printf("SAW 65Hz: old/new high-frequency derivative energy %.2f, RMS ratio %.2f, clipped old/new %d/%d\n",new_hf/old_hf,sqrt(new_rms/old_rms),oldclip,newclip);
 assert(new_hf<old_hf*0.75);assert(new_rms>old_rms*0.55);
}
static double reverb_test(int preset, int mix, double *tail){
 const int n=44100*2;
 etch_fx_reset(&fx);
 struct etch_fx_settings cfg;etch_fx_defaults(&cfg);
 cfg.reverb_size=(uint16_t)(preset==0?8500:preset==1?30000:20500);
 cfg.reverb_decay=(uint16_t)(preset==0?12000:preset==1?30000:11000);
 double energy=0,te=0;
 for(int i=0;i<n;i++){
  int16_t x=0;
  if(i<11025) x=(int16_t)(10000.0*sin(2.0*3.141592653589793*110.0*(double)i/44100.0));
  int16_t l,r;
  etch_fx_process_stereo_masked(&fx,x,0,0,(uint16_t)(mix*32767/100),&cfg,0,&l,&r);
  if(i>=3000&&i<11025)energy+=(double)l*l;
  if(i>=11025&&i<30000)te+=(double)l*l;
 }
 *tail=sqrt(te/(30000-11025));
 return sqrt(energy/8025.0);
}
int main(void){
 saw_test();
 double t[3],w[3];
 for(int k=0;k<3;k++){
  double drytail=0;
  double dry=reverb_test(k,0,&drytail);
  w[k]=reverb_test(k,100,&t[k]);
  printf("REVERB preset %d: dry RMS %.1f, fullwet RMS %.1f, wet/dry %.3f, tail RMS %.1f\n",k,dry,w[k],w[k]/dry,t[k]);
  assert(w[k]>dry*0.55);assert(t[k]>10.0);
 }
 /* Expect genuinely varied time/texture, not identical PCM. */
 assert(fabs(t[0]-t[1])>5.0);assert(fabs(t[1]-t[2])>5.0);
 puts("PASS V1.77 sound test");return 0;
}
