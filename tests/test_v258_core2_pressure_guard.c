/* V2.0.58: deterministic and fuzzed DMA-pressure retirement decisions. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "rupture_core2_pressure_guard.h"

int main(void){
 assert(!ru_core2_pressure_should_retire(0u,1u,1u,2u));
 assert(!ru_core2_pressure_should_retire(1u,0u,1u,2u));
 assert(!ru_core2_pressure_should_retire(1u,1u,2u,2u));
 assert(ru_core2_pressure_should_retire(1u,1u,1u,2u));
 assert(ru_core2_pressure_should_retire(1u,1u,0xffffffffu,0u));
 /* A retirement cannot be triggered a second time once online is false. */
 assert(!ru_core2_pressure_should_retire(0u,1u,1u,3u));
 uint32_t seed=0x245bd64du;
 for(uint32_t i=0u;i<1000000u;i++){
  seed=seed*1664525u+1013904223u;
  uint8_t online=(uint8_t)(seed&1u),qualified=(uint8_t)((seed>>1)&1u);
  uint32_t before=seed,after=seed+((seed>>4)&3u);
  uint8_t retire=ru_core2_pressure_should_retire(online,qualified,before,after);
  assert(retire==(uint8_t)(online&&qualified&&before!=after));
 }
 puts("PASS V2.0.58: 1,000,000 CORE2 DMA-pressure trip decisions, no idle false positives");
 return 0;
}
