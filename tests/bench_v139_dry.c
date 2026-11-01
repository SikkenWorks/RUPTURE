#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"
static struct etch_native_fx fx;
int main(void){
 etch_fx_reset(&fx);struct etch_fx_settings c;etch_fx_defaults(&c);
 struct rupture_eq2_params p;rupture_eq2_defaults(&p,1);
 struct rupture_eq2_bq eq[4];uint16_t gains[4]={16384,16384,16384,16384};
 rupture_eq2_compute(&p,gains,eq);rupture_eq2_publish(&fx.eq2,eq,gains);fx.eq2_enabled=1;
 volatile uint32_t sum=0;const int loops=1000000;
 clock_t t=clock();for(int i=0;i<loops;i++){
  int16_t x=(int16_t)((i*971u)%40000u-20000),l=0,r=0;
  etch_fx_process_stereo_ex(&fx,x,0,0,0,&c,&l,&r);
  sum+=(uint16_t)l;
 }
 printf("time_ms=%.3f checksum=%u\n",1000.0*(clock()-t)/CLOCKS_PER_SEC,(unsigned)sum);
 return 0;
}
