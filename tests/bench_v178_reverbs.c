#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"
static struct etch_native_fx fx;
static double stamp(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+t.tv_nsec*1e-9;}
int main(void){
 const int N=500000;
 struct etch_fx_settings c;etch_fx_defaults(&c);
 c.reverb_size=24000;c.reverb_decay=26000;
 for(int m=0;m<3;m++){
  etch_fx_reset(&fx);
#ifdef RUPTURE_ENGINE3
  etch_fx_set_reverb_type(&c,m);
#endif
  int16_t l=0,r=0;double begin=stamp();
  for(int i=0;i<N;i++){
   int16_t x=(int16_t)((i*131u)&8191u)-4095;
   etch_fx_process_stereo_masked(&fx,x,0,0,22000,&c,0,&l,&r);
  }
  double sec=stamp()-begin;
  printf("mode %d, samples %d, ns/sample %.1f, last %d,%d\n",m,N,sec*1e9/N,l,r);
 }
}
