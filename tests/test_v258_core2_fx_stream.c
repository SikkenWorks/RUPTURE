/* V2.0.58: CPU1 can only see fully published 64-frame dry/FX snapshots. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "rupture_core2_fx_stream.h"

int main(void){
 assert(RU_C1_FX_STREAM_FRAMES==128u&&RU_C1_FX_STREAM_CHUNK==64u);
 for(uint32_t ready=0u;ready<=160u;ready++){
  uint8_t legal=(uint8_t)(ready==64u||ready==128u);
  assert(ru_c1_fx_stream_start(0u,128u,ready,0u)==legal);
  assert(!ru_c1_fx_stream_start(1u,128u,ready,0u));
  assert(!ru_c1_fx_stream_start(0u,127u,ready,0u));
  assert(!ru_c1_fx_stream_start(0u,128u,ready,1u));
 }
 for(uint32_t progress=0u;progress<=160u;progress++)
  for(uint32_t ready=0u;ready<=160u;ready++){
   uint8_t legal=(uint8_t)(progress<=128u&&progress%64u==0u&&
     ready>=progress&&ready<=128u&&ready%64u==0u);
   assert(ru_c1_fx_stream_watermark(progress,ready)==legal);
   uint8_t publication=(uint8_t)(progress>=64u&&progress<128u&&ready==progress+64u);
   assert(ru_c1_fx_stream_publish(progress,ready)==publication);
  }
 uint32_t previous=64u;
 for(uint32_t step=0u;step<1u;step++){
  assert(!ru_c1_fx_stream_publish(previous,previous));
  assert(!ru_c1_fx_stream_publish(previous,previous+63u));
  assert(ru_c1_fx_stream_publish(previous,previous+64u));
  previous+=64u;
 }
 puts("PASS V2.0.58: two 64-frame watermarks, strict 128-frame CPU1 bounds");
 return 0;
}
