/* BEND V1.86.1 K3 curve invariants; no device or audio ISR required. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_bend_curves.h"
int main(void){
 for(unsigned profile=0;profile<RU_BEND_CURVE_COUNT;profile++){
  assert(ru_bend_curve_percent(profile,0)==0);
  assert(ru_bend_curve_percent(profile,100)==100);
  assert(ru_bend_curve_percent(profile,255)==100);
  for(unsigned raw=1;raw<=100;raw++){
   const uint8_t a=ru_bend_curve_percent(profile,(uint8_t)(raw-1));
   const uint8_t b=ru_bend_curve_percent(profile,(uint8_t)raw);
   assert(b>=a && b<=100);
  }
 }
 for(unsigned raw=0;raw<=100;raw++)
  assert(ru_bend_curve_percent(RU_BEND_LINEAR,(uint8_t)raw)==raw);
 assert(ru_bend_curve_percent(RU_BEND_SOFT,50)>50);
 assert(ru_bend_curve_percent(RU_BEND_HARD,50)<50);
 assert(ru_bend_curve_percent(RU_BEND_SCURVE,50)==50);
 assert(ru_bend_curve_percent(RU_BEND_EXP,50)<ru_bend_curve_percent(RU_BEND_HARD,50));
 for(unsigned target=0;target<5;target++){
  const uint32_t factors[5]={66489u,67456u,69433u,71468u,73562u};
  for(unsigned mode=0;mode<RU_BEND_CURVE_COUNT;mode++){
   const uint32_t delta=factors[target]-65536u;
   const uint32_t low=65536u+((delta*ru_bend_curve_percent((uint8_t)mode,0u)+50u)/100u);
   const uint32_t hi=65536u+((delta*ru_bend_curve_percent((uint8_t)mode,100u)+50u)/100u);
   assert(low==65536u && hi==factors[target]);
  }
 }
 puts("PASS: all 5 bend curves monotonic and range-preserving, linear legacy parity, release 0=neutral");
 return 0;
}
