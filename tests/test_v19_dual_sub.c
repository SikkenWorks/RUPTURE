#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_subosc.h"
int main(void){
 RuSubState s1,s2;ru_sub_init(&s1,3456738,0x52398ad1);ru_sub_init(&s2,3456738,0xc3bd914f);
 assert(ru_sub_inc(80000,0,50,0)==20000);
 assert(ru_sub_inc(80000,1,50,0)==40000);
 assert(ru_sub_inc(80000,2,50,0)==80000);
 assert(ru_sub_inc(80000,3,50,0)==160000);
 assert(ru_sub_inc(80000,1,0,0)<ru_sub_inc(80000,1,100,0));
 int32_t off=0;uint64_t classic_diff=0,cxs_diff=0;
 for(int i=0;i<40000;i++){
  int32_t a=ru_sub_tick(&s1,0,1,50,0,3456738,58,50,52,68);
  int32_t b=ru_sub_tick(&s2,0,0,50,0,3456738,58,50,52,68);
  assert(a==0&&b==0);off+=ru_sub_blend(1000,a,b,s1.mix,s2.mix);
 }
 assert(off==40000000);
 for(int i=0;i<40000;i++){
  int32_t a=ru_sub_tick(&s1,2,1,48,30,3456738,58,50,52,68);
  int32_t b=ru_sub_tick(&s2,1,0,57,58,3456738,58,50,52,68);
  assert(s1.mix<=16384&&s2.mix<=16384);
  assert(ru_sub_blend(24000,a,b,s1.mix,s2.mix)<=32767);
  assert(ru_sub_blend(-24000,a,b,s1.mix,s2.mix)>=-32768);
  classic_diff+=(a<0?-a:a);cxs_diff+=(b<0?-b:b);
 }
 assert(classic_diff>100000&&cxs_diff>100000);
 assert(s1.mix==16384&&s2.mix==16384);
 for(int i=0;i<200;i++)ru_sub_tick(&s1,0,1,50,0,3456738,58,50,52,68);
 assert(s1.mix==0);
 puts("PASS V1.9: two independent CXS/classic subs, octave/detune/drift, mixing, click-free fades, OFF parity");
 return 0;
}
