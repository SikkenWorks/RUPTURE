#pragma once
#include <stdint.h>
void host_delay(uint32_t ms);
static inline void fm1_delay_ms(uint32_t ms) { host_delay(ms); }
