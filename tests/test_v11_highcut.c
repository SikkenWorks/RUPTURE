#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"
static struct etch_native_fx fx;
static double render(int freq, uint16_t air){
 struct etch_fx_settings cfg; etch_fx_defaults(&cfg);cfg.eq_hpf=air;
 etch_fx_reset(&fx);double e=0.0;
 for(int n=0;n<8000;n++){
  double wave=sin(2.0*3.14159265358979323846*(double)freq*n/44100.0);
  int16_t x=(int16_t)(10000.0*wave),l=0,r=0;
  etch_fx_process_stereo_masked(&fx,x,0,0,0,&cfg,0,&l,&r);
  if(n>=2500)e+=(double)l*(double)l;
 }
 return sqrt(e/5500.0);
}
int main(void){
 double dry_lo=render(150,0),dry_hi=render(6000,0),air_lo=render(150,16384),air_hi=render(6000,16384);
 double low_ratio=air_lo/dry_lo,high_ratio=air_hi/dry_hi;
 double cut_lo=render(150,32767)/dry_lo, cut_hi=render(6000,32767)/dry_hi;
 if(!(low_ratio>0.95 && low_ratio<1.07 && high_ratio>1.35 && high_ratio>low_ratio*1.3 && cut_hi<0.65 && cut_lo>0.94)){
  fprintf(stderr,"HIGH AIR +/- failed: boost low=%g high=%g cut low=%g high=%g\n",low_ratio,high_ratio,cut_lo,cut_hi);return 1;
 }
 printf("PASS: RUPTURE K4 signed HIGH AIR raises or lowers highs without cutting lows (150Hz %.3f, 6kHz %.3f ratios)\n",low_ratio,high_ratio);
 return 0;
}
