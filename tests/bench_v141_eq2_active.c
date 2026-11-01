#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"
static struct etch_native_fx f;
static volatile uint32_t sink;
static double now(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+1e-9*t.tv_nsec;}
int main(void){struct etch_fx_settings c;etch_fx_defaults(&c);etch_fx_reset(&f);
 struct rupture_eq2_params p;rupture_eq2_defaults(&p,1);
 uint16_t g[4]={26000,22000,28000,24000};struct rupture_eq2_bq b[4];
 p.freq[0]=50;p.freq[1]=43;p.freq[2]=65;p.freq[3]=67;p.q[0]=40;p.q[1]=67;
 rupture_eq2_compute(&p,g,b);rupture_eq2_publish(&f.eq2,b,g);f.eq2_enabled=1;
 for(int i=0;i<10000;i++)etch_fx_process_ex(&f,(int16_t)(i%4000-2000),0,0,0,&c);
 double t0=now();for(int i=0;i<600000;i++)sink+=(uint16_t)etch_fx_process_ex(&f,(int16_t)(i%4000-2000),0,0,0,&c);
 printf("active EQ2 600000 samples: %.1f ms (checksum %u)\n",1000*(now()-t0),sink);
}
