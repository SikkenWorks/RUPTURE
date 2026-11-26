/* Fail-closed CPU1 mailbox decisions with deterministic fault injection. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "rupture_core2_fx_guard.h"

static uint32_t seed=0x24411cfeu;
static uint32_t random32(void){seed=seed*1664525u+1013904223u;return seed;}

int main(void){
 assert(!ru_fx_probe_finished(127u,1u));
 assert(!ru_fx_probe_finished(128u,0u));
 assert(ru_fx_probe_finished(128u,1u));
 assert(ru_fx_probe_finished(3000000u,2000u));
 assert(ru_fx_lane_reply(12u,11u,0u,0u,64u,0u)==RU_FX_REPLY_WAIT);
 assert(ru_fx_lane_reply(12u,11u,0u,0u,64u,37u)==RU_FX_REPLY_WAIT);
 assert(ru_fx_lane_reply(12u,12u,0u,0u,64u,64u)==RU_FX_REPLY_COMPLETE);
 assert(ru_fx_lane_reply(12u,12u,0u,0u,64u,63u)==RU_FX_REPLY_CORRUPT);
 assert(ru_fx_lane_reply(12u,12u,0u,0u,64u,65u)==RU_FX_REPLY_CORRUPT);
 assert(ru_fx_lane_reply(12u,13u,0u,0u,64u,64u)==RU_FX_REPLY_CORRUPT);
 assert(ru_fx_lane_reply(12u,10u,0u,0u,64u,64u)==RU_FX_REPLY_CORRUPT);
 assert(ru_fx_lane_reply(12u,12u,1u,0u,64u,64u)==RU_FX_REPLY_CORRUPT);
 assert(ru_fx_lane_reply(12u,12u,0u,64u,64u,64u)==RU_FX_REPLY_CORRUPT);
 assert(ru_fx_safe_progress(0u,64u,37u)==37u);
 assert(ru_fx_safe_progress(64u,128u,90u)==90u);
 assert(ru_fx_safe_progress(64u,128u,0xffffffffu)==64u);
 assert(ru_fx_safe_progress(0u,64u,0xffffffffu)==0u);
 assert(ru_fx_safe_progress(64u,128u,0u)==64u);
 /* Simulate a million corrupted/out-of-order mailboxes. Never accept a
  * COMPLETE indication without exact sequence and final frame count. */
 for(uint32_t i=0;i<1000000u;i++){
  uint32_t seq=random32()|1u;
  uint32_t begin=(random32()&1u)?0u:64u,end=begin+64u;
  uint32_t done=seq+(random32()%5u)-2u;
  uint32_t progress=begin+(random32()%67u);
  uint32_t bad=(random32()%41u)==0u;
  uint8_t reply=ru_fx_lane_reply(seq,done,bad,begin,end,progress);
  if(reply==RU_FX_REPLY_COMPLETE)
   assert(done==seq&&!bad&&progress==end);
  if(reply==RU_FX_REPLY_WAIT)
   assert(done==seq-1u&&!bad);
  uint32_t safe=ru_fx_safe_progress(begin,end,progress);
  assert(safe>=begin&&safe<=end);
 }
 puts("PASS V2.0.58: 1,000,000 forged CPU1 replies fail closed, deterministic partial fallback");
 return 0;
}
