/* SPDX-License-Identifier: GPL-3.0-only
 * RUPTURE V2.0.44: portable, deterministic control-plane checks for CPU1 FX.
 * No extra FX PCM operations or new buffers. Tested on host with injected
 * stalled, corrupt, partial and out-of-order mailbox replies.
 */
#ifndef RUPTURE_CORE2_FX_GUARD_H
#define RUPTURE_CORE2_FX_GUARD_H
#include <stdint.h>

enum {RU_FX_REPLY_WAIT=0u,RU_FX_REPLY_COMPLETE=1u,RU_FX_REPLY_CORRUPT=2u};

/* After the one-time 128 SRAM integrity challenges + a FIR challenge,
 * the FX worker owns CPU1 exclusively. Foreground probes used to compete
 * for the very same core during realtime playback at 20ms intervals. */
static inline uint8_t ru_fx_probe_finished(uint32_t integrity,uint32_t fir){
 return (uint8_t)(integrity>=128u&&fir>=1u);
}

/* Never trust DONE without the exact final frame index. A corrupted or
 * prematurely published DONE previously made CPU0 skip unfinished FX. */
static inline uint8_t ru_fx_lane_reply(uint32_t expected_seq,uint32_t done,
 uint32_t bad,uint32_t begin,uint32_t end,uint32_t progress){
 if(bad||begin>=end||end>128u)return RU_FX_REPLY_CORRUPT;
 if(done==expected_seq)return progress==end?
  RU_FX_REPLY_COMPLETE:RU_FX_REPLY_CORRUPT;
 return done==expected_seq-1u?RU_FX_REPLY_WAIT:RU_FX_REPLY_CORRUPT;
}

/* The last confirmed frame can be safely reused ONLY after core1 halts. */
static inline uint32_t ru_fx_safe_progress(uint32_t begin,uint32_t end,
 uint32_t progress){
 return begin<end&&end<=128u&&progress>=begin&&progress<=end?
  progress:begin;
}
#endif
