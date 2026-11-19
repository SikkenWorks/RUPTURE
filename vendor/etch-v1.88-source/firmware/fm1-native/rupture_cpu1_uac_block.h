/* SPDX-License-Identifier: GPL-3.0-only
 * EXPERIMENTAL: dedicated CPU1 44.1->48k FIR job, no direct access to the
 * live USB ring, FIR history, DMA or sound engine. Include after uac_fir.h.
 * A successful CPU1 job is NOT by itself proof of sample-exact audio: the
 * CPU0 controller must independently validate it before any PCM is used.
 */
#pragma once
#include <stdint.h>
#include "rupture_cpu1_protocol.h"

#define RU_C1_UAC_IN 64u
#define RU_C1_UAC_HIST UAC_FIR_N
#define RU_C1_UAC_OUT 72u
#define RU_C1_UAC_TAG 0x4aa4d813u
#define RU_C1_UAC_RS_H 32u

typedef struct {
 volatile uint32_t seq,phase,seed,check;
 volatile uint32_t input[RU_C1_UAC_IN];
 volatile uint32_t history[RU_C1_UAC_HIST];
 volatile uint32_t output[RU_C1_UAC_OUT];
 volatile uint32_t count,next_phase,output_hash,done,bad;
} ru_cpu1_uac_box_t;

static inline __attribute__((always_inline)) uint32_t
ru_cpu1_uac_input_hash(const ru_cpu1_uac_box_t *m,uint32_t seq)
{
 uint32_t v=RU_C1_UAC_TAG^seq^m->seed^m->phase;
 for(uint32_t i=0u;i<RU_C1_UAC_HIST;i++)v=ru_cpu1_fold(v,m->history[i],i);
 for(uint32_t i=0u;i<RU_C1_UAC_IN;i++)v=ru_cpu1_fold(v,m->input[i],i+RU_C1_UAC_HIST);
 return v;
}

static inline __attribute__((always_inline)) int32_t
ru_cpu1_uac_clip(int64_t x)
{
 x=(x+(1LL<<(UAC_FIR_Q-1)))>>UAC_FIR_Q;
 return x>32767?32767:x< -32768?-32768:(int32_t)x;
}

/* Core1 works on a copied 24-sample window and 64 incoming samples. Exactly
 * the same phase iteration, coefficient order, 64-bit sums, Q30 rounding
 * and signed-16 saturation as the existing USB uac_tap48(). */
static inline __attribute__((always_inline)) int
ru_cpu1_uac_process(ru_cpu1_uac_box_t *m,uint32_t seq)
{
 if(!seq||m->seq!=seq||m->phase>=UAC_FIR_L||
    m->check!=ru_cpu1_uac_input_hash(m,seq))return 0;
 uint32_t hist[RU_C1_UAC_HIST];
 for(uint32_t j=0u;j<RU_C1_UAC_HIST;j++)hist[j]=m->history[j];
 uint32_t ph=m->phase,wr=0u,sum=RU_C1_UAC_TAG;
 for(uint32_t i=0u;i<RU_C1_UAC_IN;i++){
  hist[i%RU_C1_UAC_HIST]=m->input[i];
  uint32_t s=(i+1u)%RU_C1_UAC_HIST;
  do{
   if(wr>=RU_C1_UAC_OUT)return 0;
   int64_t l=0,r=0;
   for(uint32_t j=0u;j<UAC_FIR_N;j++){
    uint32_t lr=hist[(s+j)%RU_C1_UAC_HIST];
    uint32_t tap_idx=ph<UAC_FIR_L/2u?j:(UAC_FIR_N-1u-j);
    uint32_t phase_idx=ph<UAC_FIR_L/2u?ph:(UAC_FIR_L-1u-ph);
    int32_t t=UAC_FIR[phase_idx][tap_idx];
    l+=(int64_t)t*(int16_t)lr;
    r+=(int64_t)t*(int16_t)(lr>>16u);
   }
   uint32_t out=(uint32_t)(uint16_t)ru_cpu1_uac_clip(l)|
                 (uint32_t)(uint16_t)ru_cpu1_uac_clip(r)<<16u;
   m->output[wr]=out;
   sum=ru_cpu1_fold(sum,out,wr);
   wr++;
   ph+=UAC_FIR_M;
  }while(ph<UAC_FIR_L);
  ph-=UAC_FIR_L;
 }
 m->count=wr;
 m->next_phase=ph;
 m->output_hash=sum;
 return 1;
}

static inline __attribute__((always_inline)) uint32_t
ru_cpu1_uac_expected_shape(uint32_t phase,uint32_t *next)
{
 uint32_t count=0u;
 for(uint32_t i=0u;i<RU_C1_UAC_IN;i++){
  do{count++;phase+=UAC_FIR_M;}while(phase<UAC_FIR_L);
  phase-=UAC_FIR_L;
 }
 *next=phase;
 return count;
}

/* Reconstruct one output independently, from the pre-block window and
 * input at position i. This spot-check is cheaper than a full FIR pass.
 * It is NOT a mathematical guarantee against an isolated bad PCM sample. */
static inline __attribute__((always_inline)) uint32_t
ru_cpu1_uac_expected_one(const ru_cpu1_uac_box_t *m,uint32_t target)
{
 uint32_t ph=m->phase,k=0u;
 for(uint32_t i=0u;i<RU_C1_UAC_IN;i++){
  do{
   if(k++==target){
    int64_t l=0,r=0;
    for(uint32_t j=0u;j<UAC_FIR_N;j++){
     int32_t index=(int32_t)i-(int32_t)(UAC_FIR_N-1u)+(int32_t)j;
     uint32_t lr=index<0?m->history[RU_C1_UAC_HIST+(uint32_t)index]:m->input[(uint32_t)index];
     int32_t t=ph<UAC_FIR_L/2u?UAC_FIR[ph][j]:
               UAC_FIR[UAC_FIR_L-1u-ph][UAC_FIR_N-1u-j];
     l+=(int64_t)t*(int16_t)lr;
     r+=(int64_t)t*(int16_t)(lr>>16u);
    }
    return (uint32_t)(uint16_t)ru_cpu1_uac_clip(l)|
           (uint32_t)(uint16_t)ru_cpu1_uac_clip(r)<<16u;
   }
   ph+=UAC_FIR_M;
  }while(ph<UAC_FIR_L);
  ph-=UAC_FIR_L;
 }
 return 0xffffffffu;
}

/* No heavy DSP here. Validate all input and output ownership plus shape.
 * The optional full PCM comparison during probation is performed by the
 * original CPU0 uac_tap48() before any result is trusted. */
static inline __attribute__((always_inline)) int
ru_cpu1_uac_verify_envelope(const ru_cpu1_uac_box_t *m,uint32_t seq,
                             const uint32_t input[RU_C1_UAC_IN],
                             const uint32_t history[RU_C1_UAC_HIST],uint32_t phase)
{
 if(!seq||m->seq!=seq||m->done!=seq||m->bad||m->phase!=phase||
    m->check!=ru_cpu1_uac_input_hash(m,seq))return 0;
 for(uint32_t i=0u;i<RU_C1_UAC_IN;i++)if(m->input[i]!=input[i])return 0;
 for(uint32_t i=0u;i<RU_C1_UAC_HIST;i++)if(m->history[i]!=history[i])return 0;
 uint32_t expected_ph;
 uint32_t n=ru_cpu1_uac_expected_shape(phase,&expected_ph);
 if(m->count!=n||n>RU_C1_UAC_OUT||m->next_phase!=expected_ph)return 0;
 uint32_t hash=RU_C1_UAC_TAG;
 for(uint32_t i=0u;i<n;i++)hash=ru_cpu1_fold(hash,m->output[i],i);
 return hash==m->output_hash;
}

/* Cheap exact state commit: the original circular history writes for every
 * input sample, without any FIR multiply. Shared with the host parity test. */
static inline void ru_cpu1_uac_apply_history(
 int16_t *left,int16_t *right,uint32_t *w,uint32_t *phase,
 const uint32_t input[RU_C1_UAC_IN],uint32_t next_phase)
{
 for(uint32_t i=0u;i<RU_C1_UAC_IN;i++){
  uint32_t idx=(*w+i)&(RU_C1_UAC_RS_H-1u);
  uint32_t lr=input[i];
  left[idx]=left[idx+RU_C1_UAC_RS_H]=(int16_t)lr;
  right[idx]=right[idx+RU_C1_UAC_RS_H]=(int16_t)(lr>>16u);
 }
 *w+=RU_C1_UAC_IN;
 *phase=next_phase;
}
