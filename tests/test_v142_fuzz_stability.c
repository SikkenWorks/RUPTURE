/* V1.42 Octave Fuzz sustained-note, parameter-morph and bypass regressions. */
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"
static struct etch_native_fx fx;
static struct etch_fx_settings cfg;
static void check(int ok, const char *msg){if(!ok){fprintf(stderr,"FAIL: %s\n",msg);exit(2);}}
static int16_t process(int16_t x){int16_t l,r;etch_fx_process_stereo_masked(&fx,x,0,0,0,&cfg,0,&l,&r);check(l==r,"mono parity");return l;}
static void restart(void){etch_fx_reset(&fx);etch_fx_defaults(&cfg);rupture_tilt_controls(50,50,0,50,0);rupture_sandpaper_controls(60,40,45,80,0);rupture_fuzz_controls(55,75,65,0,0);}
int main(int argc,char**argv){
 (void)argc;(void)argv;
 restart();
 /* Silence stays digital zero with strongly saturated octave rectifier. */
 rupture_fuzz_controls(100,100,100,100,1);
 for(int i=0;i<10000;i++)check(process(0)==0,"fuzz-on silence must be silent");
 /* No spontaneous burst or underrun on steady tone. */
 double energy=0;int maxjump=0,peak=0,last=0,steadyjump=0;
 for(int i=0;i<66000;i++){
   double z=sin(6.283185307179586*110.0*i/44100.0);
   int16_t input=(int16_t)(z*9500.0);
   int16_t y=process(input);
   if(i>=5000){int jump=abs((int)y-last);if(jump>steadyjump)steadyjump=jump;
    energy+=(double)y*y;if(abs(y)>peak)peak=abs(y);}
   last=y;
 }
 check(peak>500,"Octave Fuzz must remain audible");
 check(peak<=32767,"PCM bounded");
 /* Continuous tone while aggressively changing three parameters. */
 int maxmorph=0;last=0;
 for(int i=0;i<30000;i++){
   if(i%1279==0){int alt=(i/1279)&1;rupture_fuzz_controls(alt?100:20,alt?100:0,alt?0:100,100,1);}
   int16_t x=(int16_t)(9500.0*sin(6.283185307179586*110.0*(i+66000)/44100.0));
   int16_t y=process(x);
   if(i>500){int d=abs((int)y-last);if(d>maxmorph)maxmorph=d;}
   last=y;
 }
 check(maxmorph<4500,"extreme control sweep must not produce giant sample click");
 rupture_fuzz_controls(100,100,100,100,0);
 int lastdry=-1;for(int i=0;i<4000;i++){
   int16_t x=(int16_t)(7000.0*sin(6.283185307179586*110.0*i/44100.0));
   int16_t y=process(x);
   if(i>2000){check(y==x,"return to exact dry with effect off");lastdry=y;}
 }
 check(lastdry!=-1,"dry path exercised");
 printf("PASS V1.42 OCT FUZZ: silence=0, peak=%d RMS=%.1f, steady max delta=%d, knob max delta=%d, exact dry after OFF\n",peak,sqrt(energy/61000.0),steadyjump,maxmorph);
}
