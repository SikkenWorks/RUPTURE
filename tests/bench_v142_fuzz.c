#define _POSIX_C_SOURCE 199309L
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"
static struct etch_native_fx fx;
static struct etch_fx_settings c;
static int16_t inputs[2048];
static double now(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+t.tv_nsec*1e-9;}
int main(){etch_fx_reset(&fx);etch_fx_defaults(&c);rupture_fuzz_controls(100,100,100,100,1);
 for(int i=0;i<2048;i++)inputs[i]=(int16_t)(12000*sin(i*6.283185307179586*137/44100));
 int32_t crc=0; double start=now();
 for(int i=0;i<3000000;i++){int16_t l,r;etch_fx_process_stereo_masked(&fx,inputs[i&2047],0,0,0,&c,0,&l,&r);crc+=l;}
 printf("3000000 fuzz samples %.2f ms checksum=%d\n",(now()-start)*1000,crc);
}
