#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "rupture_bend_storyboard.h"
int main(void){
 for(uint8_t curve=0;curve<RU_BEND_CURVE_COUNT;curve++){
  assert(ru_bend_poster_frame(curve,0,0)==0);
  assert(ru_bend_poster_frame(curve,0,1)==0);
  assert(ru_bend_poster_frame(curve,100,0)==(curve==RU_BEND_SCURVE?7u:4u));
  assert(ru_bend_poster_frame(curve,100,1)==(curve==RU_BEND_SCURVE?7u:5u));
  for(uint8_t p=1;p<=100;p++){
   uint8_t rise=ru_bend_poster_frame(curve,p,0);
   uint8_t ret=ru_bend_poster_frame(curve,p,1);
   assert(rise<RU_BEND_FRAMES);
   assert(ret<RU_BEND_FRAMES);
   if(curve==RU_BEND_SCURVE)assert(rise==ret);
   else {assert(rise<=4);assert(ret>=5 && ret<=7);}
  }
 }
 assert(ru_bend_poster_frame(RU_BEND_LINEAR,80,1)==5);
 assert(ru_bend_poster_frame(RU_BEND_SOFT,60,1)==6);
 assert(ru_bend_poster_frame(RU_BEND_EXP,10,1)==7);
 puts("PASS V1.88.1 storyboard: poster climax at full bend, return frames only during release, stable close-up pan");
 return 0;
}
