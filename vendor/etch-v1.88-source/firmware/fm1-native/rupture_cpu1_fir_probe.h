/* SPDX-License-Identifier: GPL-3.0-only
 * Experimental CPU1 shadow FIR qualification. The PCM output never enters
 * USB, ALNK0 or any sound buffer. Includes no copies of DSP/FIR coefficients.
 * Include AFTER uac_fir.h: UAC_FIR is the exact production 24-tap table.
 */
#pragma once
#include <stdint.h>
#include "rupture_cpu1_protocol.h"

#define RU_C1_FIR_CHECK 0x6fc91d82u
#define RU_C1_FIR_PHASE_STEP 37u

static inline __attribute__((always_inline)) uint32_t
ru_cpu1_fir_check(uint32_t seq,uint32_t seed)
{
 return ru_cpu1_descriptor_check(seq,seed)^RU_C1_FIR_CHECK;
}

static inline __attribute__((always_inline)) int32_t ru_cpu1_fir_clip(int64_t v)
{
 v=(v+(1LL<<(UAC_FIR_Q-1)))>>UAC_FIR_Q;
 return v>32767?32767:v<(-32768)?-32768:(int32_t)v;
}

/* On CPU1: read only its private mailbox and the original, read-only XIP FIR
 * coefficients. Every incorrect input word or coefficient ultimately causes
 * a CPU0 mismatch; no inferred checksum can authorize a PCM commit. */
static inline __attribute__((always_inline)) int
ru_cpu1_fir_process(ru_cpu1_mailbox_t *m,uint32_t seq)
{
 uint32_t seed=m->seed;
 if(!seq||m->seq!=seq||m->kind!=RU_C1_JOB_FIR||
    m->check!=ru_cpu1_fir_check(seq,seed))return 0;
 for(uint32_t i=0u;i<RU_C1_WORDS;i++)
  if(m->input[i]!=ru_cpu1_expected_input(seed,i))return 0;
 uint32_t sum=RU_C1_MAGIC;
 for(uint32_t i=0u;i<RU_C1_WORDS;i++){
  uint32_t ph=(seed%UAC_FIR_L+i*RU_C1_FIR_PHASE_STEP)%UAC_FIR_L;
  uint32_t s=(seed+i)&(RU_C1_WORDS-1u);
  const int32_t *c=ph<UAC_FIR_L/2u?UAC_FIR[ph]:
                    &UAC_FIR[UAC_FIR_L-1u-ph][UAC_FIR_N-1u];
  int64_t al=0,ar=0;
  for(uint32_t j=0u;j<UAC_FIR_N;j++){
   uint32_t lr=m->input[(s+j)&(RU_C1_WORDS-1u)];
   int32_t t=*c;
   c+=ph<UAC_FIR_L/2u?1:-1;
   al+=(int64_t)t*(int16_t)lr;
   ar+=(int64_t)t*(int16_t)(lr>>16u);
  }
  uint32_t packed=(uint32_t)(uint16_t)ru_cpu1_fir_clip(al)|
                  (uint32_t)(uint16_t)ru_cpu1_fir_clip(ar)<<16u;
  m->output[i]=packed;
  sum=ru_cpu1_fold(sum,packed,i);
 }
 m->result=sum;
 return 1;
}

/* CPU0 oracle reconstructs ALL input samples from the seed and indexes the
 * production phase table independently of CPU1's pointer-based traversal.
 * A corrupt but self-consistent CPU1 result checksum is always rejected. */
static inline __attribute__((always_inline)) int
ru_cpu1_fir_verify(const ru_cpu1_mailbox_t *m,uint32_t seq)
{
 uint32_t seed=m->seed;
 if(!seq||m->done!=seq||m->seq!=seq||m->kind!=RU_C1_JOB_FIR||
    m->check!=ru_cpu1_fir_check(seq,seed)||m->bad)return 0;
 for(uint32_t i=0u;i<RU_C1_WORDS;i++)
  if(m->input[i]!=ru_cpu1_expected_input(seed,i))return 0;
 uint32_t sum=RU_C1_MAGIC;
 for(uint32_t i=0u;i<RU_C1_WORDS;i++){
  uint32_t ph=(seed%UAC_FIR_L+i*RU_C1_FIR_PHASE_STEP)%UAC_FIR_L;
  uint32_t start=(seed+i)&(RU_C1_WORDS-1u);
  int64_t l=0,r=0;
  for(uint32_t j=0u;j<UAC_FIR_N;j++){
   uint32_t lr=ru_cpu1_expected_input(seed,(start+j)&(RU_C1_WORDS-1u));
   int32_t tap=ph<UAC_FIR_L/2u?UAC_FIR[ph][j]:
                UAC_FIR[UAC_FIR_L-1u-ph][UAC_FIR_N-1u-j];
   l+=(int64_t)tap*(int16_t)lr;
   r+=(int64_t)tap*(int16_t)(lr>>16u);
  }
  uint32_t packed=(uint32_t)(uint16_t)ru_cpu1_fir_clip(l)|
                  (uint32_t)(uint16_t)ru_cpu1_fir_clip(r)<<16u;
  if(m->output[i]!=packed)return 0;
  sum=ru_cpu1_fold(sum,packed,i);
 }
 return sum==m->result;
}
