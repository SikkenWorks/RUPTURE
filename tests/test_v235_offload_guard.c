/* Portable CPU1 live USB offload fault policy, no FM1 hardware required. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "rupture_cpu1_offload_guard.h"

int main(void)
{
 ru_cpu1_pressure_guard_t g={0};
 assert(!ru_cpu1_pressure_update(&g,0,3,4,5,6,0));
 assert(!g.armed);
 assert(!ru_cpu1_pressure_update(&g,1,3,4,5,6,0));
 assert(g.armed);
 assert(!ru_cpu1_pressure_update(&g,1,3,4,5,6,0));
 assert(ru_cpu1_pressure_update(&g,1,4,4,5,6,0)==RU_C1_PRESS_DMA_LATE);
 assert(ru_cpu1_pressure_update(&g,1,4,5,5,6,0)==RU_C1_PRESS_DMA_GUARD);
 assert(ru_cpu1_pressure_update(&g,1,4,5,6,6,0)==RU_C1_PRESS_USB_UND);
 assert(ru_cpu1_pressure_update(&g,1,4,5,6,7,0)==RU_C1_PRESS_USB_STAGE);
 assert(ru_cpu1_pressure_update(&g,1,4,5,6,7,1)==RU_C1_PRESS_AUDIO_STOP);
 assert(!ru_cpu1_pressure_update(&g,1,4,5,6,7,1));
 assert(ru_cpu1_pressure_update(&g,1,5,6,7,8,0)==
   (RU_C1_PRESS_DMA_LATE|RU_C1_PRESS_DMA_GUARD|
    RU_C1_PRESS_USB_UND|RU_C1_PRESS_USB_STAGE));
 assert(!ru_cpu1_pressure_update(&g,0,0,0,0,0,0));
 assert(!ru_cpu1_pressure_update(&g,1,0,0,0,0,0));
 assert(ru_cpu1_pressure_update(&g,1,UINT32_MAX,0,0,0,0)==RU_C1_PRESS_DMA_LATE);
 assert(ru_cpu1_pressure_update(&g,1,0,0,0,0,0)==RU_C1_PRESS_DMA_LATE);

 assert(ru_cpu1_offload_decide(0,0,RU_C1_UAC_MIN_RING_FRAMES)==RU_C1_WAIT);
 assert(ru_cpu1_offload_decide(0,RU_C1_UAC_DEADLINE_TICKS,
   RU_C1_UAC_MIN_RING_FRAMES)==RU_C1_WAIT);
 assert(ru_cpu1_offload_decide(0,0,RU_C1_UAC_MIN_RING_FRAMES-1u)==RU_C1_ABORT_BUFFER);
 assert(ru_cpu1_offload_decide(1,0,RU_C1_UAC_MIN_RING_FRAMES-1u)==RU_C1_ACCEPT);
 assert(ru_cpu1_offload_decide(1,RU_C1_UAC_DEADLINE_TICKS,
   RU_C1_UAC_MIN_RING_FRAMES)==RU_C1_ACCEPT);
 assert(ru_cpu1_offload_decide(0,RU_C1_UAC_DEADLINE_TICKS+1u,
   RU_C1_UAC_MIN_RING_FRAMES)==RU_C1_ABORT_LATE);
 assert(ru_cpu1_offload_decide(1,RU_C1_UAC_DEADLINE_TICKS+1u,
   RU_C1_UAC_MIN_RING_FRAMES)==RU_C1_ABORT_LATE);
 assert(ru_cpu1_offload_decide(0,RU_C1_UAC_DEADLINE_TICKS+1u,0)==RU_C1_ABORT_LATE);
 puts("PASS V2.0.35: CPU1 pressure trip and deadline/ring fallback policy");
 return 0;
}
