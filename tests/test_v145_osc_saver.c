#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_bandwaves.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_sub_state.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_main_state.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_subosc.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_saver_loop.h"
int main(void){
 struct etch_preset_dsp v={0};
 uint8_t type[2]={3,4},oct[2]={1,0},det[2]={51,49},drift[2]={20,30};
 assert(!ru_main_state_restore(&v));
 ru_main_state_store(&v,1u);
 ru_sub_state_store(&v,type,oct,det,drift);
 assert(ru_main_state_restore(&v));
 assert(ru_sub_state_valid(&v));
 uint8_t rt[2],ro[2],rd[2],rr[2];
 assert(ru_sub_state_restore(&v,rt,ro,rd,rr));
 assert(rt[0]==3&&rt[1]==4&&ro[1]==0&&rd[0]==51&&rr[1]==30);
 ru_main_state_store(&v,0u);
 assert(!ru_main_state_restore(&v));
 /* Bound every mip-band / phase. No overflow, zero mean, non-silent. */
 for(int wave=3;wave<=4;wave++)for(int band=0;band<7;band++){
  uint32_t inc=0x7fffffffu>>(band+1);
  int64_t sum=0,energy=0;
  for(int i=0;i<256;i++){
   int32_t x=ru_bandwave_read((uint8_t)wave,(uint32_t)i<<24,inc);
   assert(x>=-30000&&x<=30000);
   sum+=x;energy+=((int64_t)x*x);
  }
  assert(llabs(sum)<512 && energy>100000000);
 }
 RuSubState a,b;ru_sub_init(&a,6000000u,1u);ru_sub_init(&b,6000000u,2u);
 int64_t sum_a=0,sum_b=0;
 for(int i=0;i<110000;i++){
  int32_t sa=ru_sub_tick(&a,3u,1u,50u,20u,6000000u,58u,50u,52u,68u);
  int32_t sb=ru_sub_tick(&b,4u,0u,55u,30u,6000000u,58u,50u,52u,68u);
  assert(sa>=-32768&&sa<=32767); assert(sb>=-32768&&sb<=32767);
  sum_a+=(int64_t)sa*sa;sum_b+=(int64_t)sb*sb;
 }
 assert(sum_a>100000000&&sum_b>100000000);
 /* Both new types fade down before switching to an older type. */
 for(int i=0;i<80;i++)ru_sub_tick(&a,1u,1u,50u,20u,6000000u,58u,50u,52u,68u);
 assert(a.running_type==1u && a.mix>0u);
 /* End-of-animation pause, reverse playback and continuous repeat. */
 uint8_t frame=0,reverse=0;uint32_t hold=0,now=0;
 for(int i=0;i<63;i++){
  now+=150;assert(ru_saver_step(&frame,&reverse,&hold,now)==1u);
 }
 assert(frame==63 && hold==now+2200 && !reverse);
 now+=2000;assert(!ru_saver_step(&frame,&reverse,&hold,now));assert(frame==63);
 now+=300;assert(!ru_saver_step(&frame,&reverse,&hold,now));assert(reverse);
 for(int i=0;i<63;i++){now+=150;assert(ru_saver_step(&frame,&reverse,&hold,now)==1u);}
 assert(frame==0 && !reverse && !hold);
 now+=150;assert(ru_saver_step(&frame,&reverse,&hold,now)==1u);assert(frame==1);
 puts("PASS V1.45: main oscillator bit, old project default, SQUARE/SAW subs, mip samples, screensaver forward/hold/reverse/loop");
 return 0;
}
