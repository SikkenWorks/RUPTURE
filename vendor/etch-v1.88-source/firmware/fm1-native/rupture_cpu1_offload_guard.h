/* SPDX-License-Identifier: GPL-3.0-only
 * CPU0-owned fail-closed observation of optional CPU1 work. Pure C, no IRQ,
 * sound or USB register access. The already-occurring underrun is not undone.
 */
#pragma once
#include <stdint.h>

#define RU_C1_UAC_DEADLINE_TICKS 36000u  /* TIMER4 24MHz: 1.5ms */
#define RU_C1_UAC_MIN_RING_FRAMES 64u   /* ~1.33ms of 48k USB PCM reserve */

enum { RU_C1_WAIT=0, RU_C1_ACCEPT=1, RU_C1_ABORT_LATE=2,
       RU_C1_ABORT_BUFFER=3 };
enum { RU_C1_PRESS_DMA_LATE=1u, RU_C1_PRESS_DMA_GUARD=2u,
       RU_C1_PRESS_USB_UND=4u, RU_C1_PRESS_USB_STAGE=8u,
       RU_C1_PRESS_AUDIO_STOP=16u };

typedef struct {
 uint32_t dma_lates,dma_guard,usb_underruns,usb_stage_drops;
 uint8_t audio_stop,armed;
} ru_cpu1_pressure_guard_t;

/* Only observe new faults during the opted-in live CPU1 recording session.
 * A host unplug or deliberate stream reset resets the baseline, not hardware. */
static inline uint32_t ru_cpu1_pressure_update(ru_cpu1_pressure_guard_t *g,
 uint8_t active,uint32_t dma_lates,uint32_t dma_guard,
 uint32_t usb_underruns,uint32_t usb_stage_drops,uint8_t audio_stop)
{
 if(!active){g->armed=0u;return 0u;}
 if(!g->armed){
  g->dma_lates=dma_lates;g->dma_guard=dma_guard;
  g->usb_underruns=usb_underruns;g->usb_stage_drops=usb_stage_drops;
  g->audio_stop=audio_stop;g->armed=1u;
  return 0u;
 }
 uint32_t flags=0u;
 if(dma_lates!=g->dma_lates)flags|=RU_C1_PRESS_DMA_LATE;
 if(dma_guard!=g->dma_guard)flags|=RU_C1_PRESS_DMA_GUARD;
 if(usb_underruns!=g->usb_underruns)flags|=RU_C1_PRESS_USB_UND;
 if(usb_stage_drops!=g->usb_stage_drops)flags|=RU_C1_PRESS_USB_STAGE;
 if(audio_stop&&!g->audio_stop)flags|=RU_C1_PRESS_AUDIO_STOP;
 g->dma_lates=dma_lates;g->dma_guard=dma_guard;
 g->usb_underruns=usb_underruns;g->usb_stage_drops=usb_stage_drops;
 g->audio_stop=audio_stop;
 return flags;
}

/* CPU0 waits only while there is enough buffered PCM AND enough time to
 * perform the original FIR as fallback. A finished-but-late result does not
 * get committed simply because its checksum is valid. */
static inline uint32_t ru_cpu1_offload_decide(uint8_t done,
 uint32_t elapsed_ticks,uint32_t ring_frames)
{
 if(elapsed_ticks>RU_C1_UAC_DEADLINE_TICKS)return RU_C1_ABORT_LATE;
 if(done)return RU_C1_ACCEPT;
 if(ring_frames<RU_C1_UAC_MIN_RING_FRAMES)return RU_C1_ABORT_BUFFER;
 return RU_C1_WAIT;
}
