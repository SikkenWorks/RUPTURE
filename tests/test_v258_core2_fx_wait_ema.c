#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "rupture_core2_fx_wait_ema.h"

int main(void)
{
 assert(ru_fx_wait_ema_next(0u,24000u,0u)==24000u);
 assert(ru_fx_wait_ema_next(24000u,0u,1u)==22500u);
 assert(ru_fx_wait_ema_next(0u,24000u,1u)==1500u);
 assert(ru_fx_wait_ema_next(12000u,12000u,1u)==12000u);

 /* More than a million bounded 1.08-ms wait patterns, with no signed
  * overflow or out-of-range result. */
 uint32_t seed=0x98212badu,avg=0u,previous=0u;
 for(uint32_t i=0u;i<1200000u;i++){
  seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;
  uint32_t waited=seed%26001u;
  uint32_t next=ru_fx_wait_ema_next(avg,waited,i!=0u);
  uint32_t lo=(i?((avg<waited)?avg:waited):waited);
  uint32_t hi=(i?((avg>waited)?avg:waited):waited);
  assert(next>=lo&&next<=hi);
  assert(next<=26000u);
  previous=avg;avg=next;
 }
 assert(avg<=26000u&&previous<=26000u);
 puts("PASS V2.0.58: 1,200,000 CORE2 FX wait EMA updates bounded");
 return 0;
}
