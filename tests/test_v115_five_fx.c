#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx_bypass_v181.h"
static struct etch_native_fx st;
static struct etch_fx_settings cfg;
static void expect(int yes,const char *name){if(!yes){fprintf(stderr,"FAIL %s\n",name);exit(2);}}
static uint64_t compare(int kind){
 etch_fx_reset(&st);etch_fx_defaults(&cfg);
 rupture_transient_controls(50,50,60,0,0);
 rupture_tilt_controls(50,50,0,50,0);
 rupture_fuzz_controls(50,75,70,0,0);
 static int16_t baseline[9000];uint64_t difference=0;
 for(int pass=0;pass<2;pass++){
  if(pass==1){
   etch_fx_reset(&st);
   if(kind==0){cfg.comp_threshold=32767;cfg.comp_ratio=28000;cfg.comp_attack=8000;cfg.comp_release=22000;}
   if(kind==1){cfg.folder_fold=13000;cfg.folder_symmetry=16384;cfg.folder_tone=32767;cfg.folder_mix=26000;}
   if(kind==2)rupture_transient_controls(100,30,100,100,1);
   if(kind==3)rupture_tilt_controls(95,50,100,50,1);
   if(kind==4)rupture_fuzz_controls(80,95,95,95,1);
  }
  for(int i=0;i<9000;i++){
   /* bass fundamental + harmonics, percussive step changes, DC balanced */
   double sample=8500*sin(2.0*3.141592653589793*i/99.0)+4200*sin(2.0*3.141592653589793*i/13.0);
   if(i%401<45)sample*=1.5;
   int16_t in=(int16_t)sample,l=0,r=0;
   etch_fx_process_stereo_masked(&st,in,0,0,0,&cfg,0,&l,&r);
   if(!pass) {baseline[i]=l;expect(l==in && r==in,"neutral default exact dry PCM");}
   else if(i>1500) difference+=(uint64_t)abs((int)l-(int)baseline[i]);
  }
 }
 rupture_transient_controls(50,50,60,0,0);
 rupture_tilt_controls(50,50,0,50,0);
 rupture_fuzz_controls(50,75,70,0,0);
 return difference;
}
int main(void){
 const char *names[]={"COMPRESSOR","WAVEFOLDER (REMOVED)","TRANSIENT (REMOVED)","TILT EQ","OCTAVE FUZZ"};
 for(int i=0;i<5;i++){
  uint64_t diff=compare(i);
  printf("%s difference=%llu\n",names[i],(unsigned long long)diff);
  if(i==1||i==2)expect(diff==0,"removed FX must be exact-bypass in RUPTURE");
  else expect(diff>25000,"active FX has audible DSP difference");
 }
 /* After bypass fades have settled, all five effects must return bit-exact dry. */
 etch_fx_reset(&st);etch_fx_defaults(&cfg);
 rupture_transient_controls(100,0,100,75,1);
 int16_t l,r;
 for(int i=0;i<2000;i++)etch_fx_process_stereo_masked(&st,(int16_t)(i%3000-1500),0,0,0,&cfg,0,&l,&r);
 rupture_transient_controls(100,0,100,75,0);
 for(int i=0;i<3500;i++)etch_fx_process_stereo_masked(&st,0,0,0,0,&cfg,0,&l,&r);
 for(int i=0;i<100;i++){
  int16_t in=(int16_t)(i*37-1850);
  etch_fx_process_stereo_masked(&st,in,0,0,0,&cfg,0,&l,&r);
  expect(l==in && r==in,"exact dry after transient bypass fade");
 }
 puts("PASS active COMP/TILT/FUZZ audible, removed FOLDER/TRANSIENT bypassed, defaults exact");
}
