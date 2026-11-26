#ifndef RUPTURE_CORE2_FX_WAIT_EMA_H
#define RUPTURE_CORE2_FX_WAIT_EMA_H

#include <stdint.h>

/* Diagnostic only: a bounded 1/16 exponential moving average of successful
 * CPU1 FX completion waits. The first successful job seeds the meter.
 * No DMA, sample, FX-state or CPU1 mailbox field is changed here. */
static inline uint32_t ru_fx_wait_ema_next(uint32_t previous,
                                            uint32_t measured,
                                            uint8_t have_previous)
{
 if(!have_previous)return measured;
 int32_t delta=(int32_t)measured-(int32_t)previous;
 return (uint32_t)((int32_t)previous+delta/16);
}

#endif
