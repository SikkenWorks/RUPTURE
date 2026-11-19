#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "uac_fir.h"
#include "rupture_cpu1_uac_block.h"

#define RS_H 32u
struct Ref {int16_t l[2u*RS_H],r[2u*RS_H];uint32_t w,ph;};
static struct Ref ref;
static struct Ref applied;
static ru_cpu1_uac_box_t box;
static uint32_t seed=0x48a575b7u;
static uint32_t rand32(void){seed=seed*1664525u+1013904223u;return seed;}

static void reference_input(uint32_t packed,uint32_t *out,uint32_t *n)
{
 uint32_t s=ref.w&(RS_H-1u);
 ref.l[s]=ref.l[s+RS_H]=(int16_t)packed;
 ref.r[s]=ref.r[s+RS_H]=(int16_t)(packed>>16u);
 ref.w++;
 s=(ref.w-UAC_FIR_N)&(RS_H-1u);
 do{
  int64_t l=0,r=0;
  const int16_t *x=&ref.l[s];
  if(ref.ph<UAC_FIR_L/2u){
   const int32_t *c=UAC_FIR[ref.ph];
   for(uint32_t j=0u;j<UAC_FIR_N;j++,x++){
    l+=(int64_t)(*c)*x[0];
    r+=(int64_t)(*c++)*x[2u*RS_H];
   }
  }else{
   const int32_t *c=&UAC_FIR[UAC_FIR_L-1u-ref.ph][UAC_FIR_N-1u];
   for(uint32_t j=0u;j<UAC_FIR_N;j++,x++){
    l+=(int64_t)(*c)*x[0];
    r+=(int64_t)(*c--)*x[2u*RS_H];
   }
  }
  out[(*n)++]=(uint32_t)(uint16_t)ru_cpu1_uac_clip(l)|
              (uint32_t)(uint16_t)ru_cpu1_uac_clip(r)<<16u;
  ref.ph+=UAC_FIR_M;
 }while(ref.ph<UAC_FIR_L);
 ref.ph-=UAC_FIR_L;
}

int main(void)
{
 uint64_t hash=UINT64_C(14695981039346656037);
 uint32_t input[RU_C1_UAC_IN],hist[RU_C1_UAC_HIST],out[RU_C1_UAC_OUT];
 uint32_t count=0u;
 for(uint32_t block=1u;block<=2048u;block++){
  memset((void *)&box,0,sizeof box);
  for(uint32_t i=0u;i<RU_C1_UAC_HIST;i++){
   uint32_t idx=(ref.w-RU_C1_UAC_HIST+i)&(RS_H-1u);
   box.history[i]=hist[i]=(uint16_t)ref.l[idx]|(uint32_t)(uint16_t)ref.r[idx]<<16u;
  }
  for(uint32_t i=0u;i<RU_C1_UAC_IN;i++){
   uint32_t s=rand32();
   if((block&31u)==0u&&i<4u)s=(i&1u)?0x80007fffu:0x7fff8000u;
   box.input[i]=input[i]=s;
  }
  uint32_t phase=ref.ph,seq=block;
  box.seq=seq;box.phase=phase;box.seed=rand32();
  box.check=ru_cpu1_uac_input_hash(&box,seq);
  assert(ru_cpu1_uac_process(&box,seq));
  assert(!ru_cpu1_uac_verify_envelope(&box,seq,input,hist,phase));
  box.done=seq;
  assert(ru_cpu1_uac_verify_envelope(&box,seq,input,hist,phase));
  uint32_t n=0u;
  for(uint32_t i=0u;i<RU_C1_UAC_IN;i++)reference_input(input[i],out,&n);
  assert(n==box.count&&ref.ph==box.next_phase);
  ru_cpu1_uac_apply_history(applied.l,applied.r,&applied.w,&applied.ph,
                            input,box.next_phase);
  assert(applied.w==ref.w&&applied.ph==ref.ph);
  assert(memcmp(applied.l,ref.l,sizeof(ref.l))==0);
  assert(memcmp(applied.r,ref.r,sizeof(ref.r))==0);
  for(uint32_t i=0u;i<n;i++){
   assert(out[i]==box.output[i]);
   assert(ru_cpu1_uac_expected_one(&box,i)==out[i]);
   hash^=out[i];hash*=UINT64_C(1099511628211);
  }
  count+=n;
  box.output[n/2u]^=0x10000u;
  assert(!ru_cpu1_uac_verify_envelope(&box,seq,input,hist,phase));
  box.output[n/2u]^=0x10000u;
  box.input[0]^=1u;
  assert(!ru_cpu1_uac_verify_envelope(&box,seq,input,hist,phase));
  box.input[0]^=1u;
  box.check^=1u;
  assert(!ru_cpu1_uac_verify_envelope(&box,seq,input,hist,phase));
 }
 printf("PASS V2.0.34 CPU1 live 64-frame FIR: %u outputs exactly match legacy, hash=%016llx\n",
   count,(unsigned long long)hash);
 return 0;
}
