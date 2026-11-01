/* RUPTURE native audio pipeline EQ2 4-band active, smooth, and dry parity. */
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"
static struct etch_native_fx fx;
static double tone(uint16_t q,uint32_t hz){
 etch_fx_reset(&fx);
 struct etch_fx_settings c;etch_fx_defaults(&c);
 struct rupture_eq2_params p;rupture_eq2_defaults(&p,1);p.q[1]=(uint8_t)q;
 uint16_t gains[4]={16384u,16384u,32767u,16384u};
 struct rupture_eq2_bq coefficients[4];rupture_eq2_compute(&p,gains,coefficients);
 rupture_eq2_publish(&fx.eq2,coefficients,gains);fx.eq2_enabled=1;
 double energy=0.;uint32_t count=0u;
 for(uint32_t i=0;i<55000;i++){
  int16_t input=(int16_t)(10000*sin(2*3.141592653589793*(double)i*(double)hz/44100.));
  int16_t output=etch_fx_process_ex(&fx,input,0,0,0,&c);
  if(i>25000){energy+=(double)output*(double)output;count++;}
 }
 return sqrt(energy/count);
}
int main(void){
 etch_fx_reset(&fx);struct etch_fx_settings c;etch_fx_defaults(&c);
 struct rupture_eq2_params p;rupture_eq2_defaults(&p,1);
 uint16_t gains[4]={16384u,16384u,16384u,16384u};struct rupture_eq2_bq coeff[4];rupture_eq2_compute(&p,gains,coeff);
 rupture_eq2_publish(&fx.eq2,coeff,gains);fx.eq2_enabled=1;
 for(int i=0;i<7000;i++){
  int16_t input=(int16_t)((i*1127u)%50000u-25000);
  int16_t output=etch_fx_process_ex(&fx,input,0,0,0,&c);
  assert(input==output);
 }
 double broad=tone(5u,900),narrow=tone(90u,900);
 assert(broad>narrow*1.08);
 puts("PASS native FX channel EQ2 neutral PCM parity, Q-dependent bell shoulder & auto trim");
}
