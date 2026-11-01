#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_audio_timing.h"
int main(void){
 const uint32_t d=128u*544u;
 assert(etch_audio_scope_may_capture(0,d,0));
 assert(etch_audio_scope_may_capture(d/2u,d,15));
 assert(!etch_audio_scope_may_capture(d*3u/4u,d,0));
 assert(!etch_audio_scope_may_capture(d/2u,d,16));
 assert(!etch_audio_scope_may_capture(0,0,0));
 assert(etch_audio_dma_preflight(1,1,0,10000,d));
 assert(!etch_audio_dma_preflight(0,1,0,10000,d));
 assert(!etch_audio_dma_preflight(1,1,0,d-4095,d));
 assert(!etch_audio_dma_preflight(1,1,0,d,d));
 struct etch_audio_timing t=etch_audio_timing_end(0xfffffff0u,0x30u,20u,d,0);
 assert(t.wall_ticks==0x40u && t.nested_ticks==20 && t.render_ticks==44 && !t.late);
 t=etch_audio_timing_end(100,100+d,0,d,0);assert(t.late);
 t=etch_audio_timing_end(100,300,999,d,1);assert(t.late && t.nested_ticks==200 && t.render_ticks==0);
 puts("PASS V1.19: DMA budget, scope shedding, half ownership, tick wrap and nested timing");return 0;
}
