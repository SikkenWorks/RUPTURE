/* SPDX-License-Identifier: GPL-3.0-only
 * V2.0.45: fail over after a real DMA deadline miss involving active core2 FX.
 * Realtime code only reads the preexisting completed-FX counter.
 */
#ifndef RUPTURE_CORE2_PRESSURE_GUARD_H
#define RUPTURE_CORE2_PRESSURE_GUARD_H
#include <stdint.h>

/* A DMA LATE unrelated to the second core must not needlessly disable it.
 * Only a block that committed real FX samples on CPU1 is eligible. This
 * handles 32-bit commit-counter wrap and does not touch DSP sample values. */
static inline uint8_t ru_core2_pressure_should_retire(uint8_t core_online,
 uint8_t qualified,uint32_t jobs_before,uint32_t jobs_after){
 return (uint8_t)(core_online && qualified && jobs_before!=jobs_after);
}
#endif
