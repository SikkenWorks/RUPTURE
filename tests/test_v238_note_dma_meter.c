/* V2.0.38: attribution never changes note/PCM state, even on overload. */
#include <assert.h>
#include <stdio.h>
#include "rupture_note_dma_meter.h"

int main(void){
 ru_note_dma_meter_t m={0};
 assert(ru_note_dma_edge(0,0,0,0,0,0,0,0)==0u);
 assert(ru_note_dma_edge(0,1,0,0,0,1,0,1)==RU_NDMA_ON);
 assert(ru_note_dma_edge(1,0,0,0,1,0,1,0)==RU_NDMA_OFF);
 assert(ru_note_dma_edge(0,0,3,4,1,1,1,1)==RU_NDMA_ON);
 assert(ru_note_dma_edge(0,0,3,3,2,1,1,1)==RU_NDMA_OFF);
 assert(ru_note_dma_edge(3,4,3,4,1,1,1,1)==(RU_NDMA_ON|RU_NDMA_OFF));
 ru_note_dma_record(&m,0u,100u,200u,1u);
 assert(m.on_events==0u&&m.off_events==0u);
 ru_note_dma_record(&m,RU_NDMA_ON,110u,310u,0u);
 ru_note_dma_record(&m,RU_NDMA_OFF,150u,450u,1u);
 ru_note_dma_record(&m,RU_NDMA_ON|RU_NDMA_OFF,160u,430u,1u);
 assert(m.on_events==2u&&m.off_events==2u);
 assert(m.on_lates==1u&&m.off_lates==2u);
 assert(m.on_wall_peak==430u&&m.off_wall_peak==450u);
 assert(m.on_render_peak==160u&&m.off_render_peak==160u);
 m.on_events=m.off_events=m.on_lates=m.off_lates=UINT32_MAX;
 ru_note_dma_record(&m,RU_NDMA_ON|RU_NDMA_OFF,0u,0u,1u);
 assert(m.on_events==UINT32_MAX&&m.off_events==UINT32_MAX);
 assert(m.on_lates==UINT32_MAX&&m.off_lates==UINT32_MAX);
 puts("PASS V2.0.38 read-only ON/OFF DMA attribution and saturation");
 return 0;
}
