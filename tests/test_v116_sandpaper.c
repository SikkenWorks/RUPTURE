/* Sandpaper end-to-end native fixed-point PCM proof; works on host, same C source on FM1. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"
static struct etch_native_fx fx;
static struct etch_fx_settings cfg;
static void check(int cond,const char *name){if(!cond){fprintf(stderr,"FAIL Sandpaper: %s\n",name);exit(3);}}
static void off(void){rupture_sandpaper_controls(60,40,45,80,0);}
/* Fundamental correlation of output / input, warm-up removed. */
static double ratio(int freq,int smooth,int soft,int warmth){
 etch_fx_reset(&fx);etch_fx_defaults(&cfg);
 rupture_sandpaper_controls(smooth,soft,warmth,100,1);
 double in_energy=0,cross=0;
 for(int i=0;i<16000;i++){
  double a=2.0*3.14159265358979323846*(double)freq*(double)i/44100.0;
  int16_t in=(int16_t)(6500*sin(a)),l,r;
  etch_fx_process_stereo_masked(&fx,in,0,0,0,&cfg,0,&l,&r);
  if(i>6000){in_energy+=(double)in*in;cross+=(double)l*in;}
 }
 off();return cross/in_energy;
}
int main(void){
 off();etch_fx_reset(&fx);etch_fx_defaults(&cfg);
 for(int i=0;i<1200;i++){int16_t in=(int16_t)(i%301-150),l,r;
  etch_fx_process_stereo_masked(&fx,in,0,0,0,&cfg,0,&l,&r);
  check(l==in&&r==in,"OFF is bit-exact dry");}
 double bass=ratio(95,85,0,0),bright=ratio(4500,85,0,0);
 printf("Sandpaper spectral ratio: 95 Hz %.3f; 4.5 kHz %.3f\n",bass,bright);
 check(bass>0.75&&bass<1.12,"bass fundamental survives smoothing");
 check(bright<0.60,"harsh upper harmonics are attenuated");
 double open=ratio(4500,0,0,0),closed=ratio(4500,100,0,0);
 check(open>closed*2.0,"SMOOTH knob has a real frequency response");
 double soft_low=ratio(95,65,0,0),soft_high=ratio(95,65,100,0);
 check(soft_high>0.7&&soft_high<1.1&&soft_low>0.7,"SOFT remains bounded");
 /* SOFT must be audible for mid-level CXS oscillations, not only near full scale. */
 etch_fx_reset(&fx);etch_fx_defaults(&cfg);
 rupture_sandpaper_controls(0,0,0,100,1);
 int64_t drypeak=0,softpeak=0;
 for(int i=0;i<8000;i++){
  int16_t in=(int16_t)(16000*sin(2.0*3.14159265358979323846*95.0*i/44100.0)),l,r;
  etch_fx_process_stereo_masked(&fx,in,0,0,0,&cfg,0,&l,&r);
  if(i>5000)drypeak+=(int32_t)abs(l);
 }
 etch_fx_reset(&fx);etch_fx_defaults(&cfg);
 rupture_sandpaper_controls(0,100,0,100,1);
 for(int i=0;i<8000;i++){
  int16_t in=(int16_t)(16000*sin(2.0*3.14159265358979323846*95.0*i/44100.0)),l,r;
  etch_fx_process_stereo_masked(&fx,in,0,0,0,&cfg,0,&l,&r);
  if(i>5000)softpeak+=(int32_t)abs(l);
 }
 check(softpeak<drypeak*9/10,"SOFT knee visibly changes upper peaks at moderate input");
 double body_off=ratio(90,65,0,0),body_on=ratio(90,65,0,100);
 check(body_on>body_off,"WARMTH restores low-body gain");
 /* Full-scale, discontinuities, and 16-bit clipping guards on both channels. */
 etch_fx_reset(&fx);etch_fx_defaults(&cfg);
 rupture_sandpaper_controls(100,100,100,100,1);
 for(int i=0;i<3000;i++){
  int16_t in=(i&1)?32767:-32768,l=0,r=0;
  etch_fx_process_stereo_masked(&fx,in,0,0,0,&cfg,0,&l,&r);
  check(l==r,"mono process produces balanced stereo");
 }
 off();for(int i=0;i<3000;i++){int16_t l,r;etch_fx_process_stereo_masked(&fx,0,0,0,0,&cfg,0,&l,&r);}
 for(int i=0;i<200;i++){int16_t in=(int16_t)(i*79-7900),l,r;
  etch_fx_process_stereo_masked(&fx,in,0,0,0,&cfg,0,&l,&r);
  check(l==in&&r==in,"bit-exact OFF after fade");}
 puts("PASS Sandpaper: spectral smoothing, bass preservation, independent knobs, warm body, bypass, bounds");
 return 0;
}
