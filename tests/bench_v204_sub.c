#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include "rupture_subosc.h"
static double seconds(void){
 struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);
 return (double)t.tv_sec+(double)t.tv_nsec*0.000000001;
}
int main(void){
 RuSubState s;ru_sub_init(&s,0x019d7e32u,0x12345678u);
 uint32_t digest=2166136261u;double t0=seconds();
 for(uint32_t i=0;i<1000000u;i++){
  uint32_t inc=0x019d7e32u;
  uint8_t detune=51u;
  uint8_t drift=0u;
  int32_t wave=ru_sub_tick(&s,RU_SUB_CXS,1u,detune,drift,inc,70u,48u,57u,66u);
  uint16_t gain=ru_sub_smooth_blend(&s,75u);
  digest=(digest^(uint32_t)(uint16_t)wave)*16777619u;
  digest=(digest^(uint32_t)gain)*16777619u;
 }
 printf("SUB PCM digest %08x; %.2f ns/sample\n",digest,
  (seconds()-t0)*1000000000.0/1000000.0);
 return 0;
}
