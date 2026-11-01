/* RUPTURE V1.22: post-mix quality, legacy effect removal and sub blend UI. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_fx_memory.h"
static struct etch_native_fx fx;
static struct etch_fx_settings cfg;
static void require(int test, const char *msg){if(!test){fprintf(stderr,"V1.22 FAIL %s\n",msg);exit(2);}}
static double level(int kind,int control,int val,int freq){
 etch_fx_reset(&fx);etch_fx_defaults(&cfg);
 rupture_tilt_controls(50,50,0,50,0);
 rupture_fuzz_controls(55,75,65,65,0);
 rupture_sandpaper_controls(60,40,45,80,0);
 if(kind==1){int a=75,f=50,m=100,o=50;
  if(control==0)a=val; if(control==1){a=85;f=val;} if(control==2){a=85;m=val;} if(control==3){a=85;o=val;}
  rupture_tilt_controls(a,f,m,o,1);
 }
 if(kind==2)rupture_fuzz_controls(100,100,100,100,1);
 if(kind==3){int sm=70,so=45,wa=40,mi=100;
  if(control==1)so=val;if(control==2)wa=val;if(control==0)sm=val;
  rupture_sandpaper_controls(sm,so,wa,mi,1);
 }
 double energy=0;
 for(int i=0;i<16000;i++){
  double w=0.7*sin(6.283185307179586*(double)freq*i/44100.0)+0.3*sin(6.283185307179586*2.1*(double)freq*i/44100.0);
  int16_t in=(int16_t)(10000*w),l,r;
  etch_fx_process_stereo_masked(&fx,in,0,0,0,&cfg,0,&l,&r);
  if(i>=8000)energy+=(double)l*l;

 }
 
 return sqrt(energy/8000.0);
}
int main(void){
 /* Four UI parameters genuinely affect PCM at useful frequencies. */
 double t1=level(1,0,5,700),t2=level(1,0,95,700);
 require(fabs(t1-t2)>300,"TILT balance audible");
 double f1=level(1,1,0,2500),f2=level(1,1,100,2500);
 require(fabs(f1-f2)>150,"TILT crossover frequency audible");
 double m1=level(1,2,0,700),m2=level(1,2,100,700);
 require(fabs(m1-m2)>300,"TILT mix audible");
 double o1=level(1,3,0,700),o2=level(1,3,100,700);
 require(fabs(o1-o2)>300,"TILT output audible");
 double s1=level(3,1,0,200),s2=level(3,1,100,200);
 double w1=level(3,2,0,120),w2=level(3,2,100,120);
 require(fabs(s1-s2)>180,"SANDPAPER soft audible");
 require(fabs(w1-w2)>250,"SANDPAPER warmth audible");
 /* Loud 100% drive/octave fuzz shouldn't multiply original loudness. */
 double dry=level(0,0,0,140),fuzz=level(2,0,0,140);
 
 require(fuzz<dry*1.32,"OCTAVE FUZZ must normalize hot wet signal");
 /* Legacy records can still decode; writing blend marks only the spare bit. */
 struct rupture_fx_memory record;ru_fxm_defaults(&record);
 record.knobs[0][0]=24;record.knobs[0][1]=88;record.enabled=0x10;
 require(ru_fxm_valid(&record),"PJT33 tag accepts BLEND");
 record.enabled=0x30;
 require(ru_fxm_valid(&record),"PJT33 accepts signed AIR marker");
 record.enabled=0x40;
 require(!ru_fxm_valid(&record),"PJT33 rejects unknown flags");
 /* Sandpaper rapid control changes must stay bounded and not fall silent. */
 etch_fx_reset(&fx);etch_fx_defaults(&cfg);rupture_sandpaper_controls(60,40,45,100,1);
 int32_t maxjump=0,last=0;int32_t peak=0;
 for(int i=0;i<22000;i++){
  if(!(i%353))rupture_sandpaper_controls((i/353)&1?100:0,(i/353)&1?100:0,(i/353)&1?100:0,100,1);
  int16_t x=(int16_t)(11000*sin(6.283185307179586*125.0*i/44100.0)),l,r;
  etch_fx_process_stereo_masked(&fx,x,0,0,0,&cfg,0,&l,&r);
  int32_t d=abs((int)l-last);if(d>maxjump)maxjump=d;
  if(abs((int)l)>peak)peak=abs((int)l);
  last=l;
 }
 require(peak<28001,"sandpaper peak guarded");
 require(maxjump<4500,"sandpaper fast knob changes no sharp jumps");
 printf("PASS V1.22 TILT K1-4, SAND SOFT/WARMTH, FUZZ loudness dry=%.0f wet=%.0f, knob jump=%d, PJT33 BLEND tag\n",dry,fuzz,(int)maxjump);
}
