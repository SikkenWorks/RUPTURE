/* Exact UAC 48k PCM regression for the packed-only foreground FIR split.
 * This is host arithmetic parity, not real FM1 USB/DMA timing validation. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/uac_fir.h"

#define HALF 128u
#define STAGE_N 1024u
#define BLOCKS 512u
#define MAX_OUTPUT (BLOCKS*140u+1024u)
#define RS_H 32u

static uint32_t seed=0x7bd1c4e5u;
static uint32_t rnd(void){seed=seed*1664525u+1013904223u;return seed;}
static int32_t clip16(int32_t v){return v>32767?32767:v< -32768?-32768:v;}

struct Fir {
 int16_t l[RS_H*2u],r[RS_H*2u];
 uint32_t w,ph;
};
static struct Fir reference,candidate;
static uint32_t reference_pcm[MAX_OUTPUT],candidate_pcm[MAX_OUTPUT];
static uint32_t nr,nc;
static uint32_t stage[STAGE_N],sw,sr,stage_high;
static uint32_t transfer[HALF];

static void fir_input(struct Fir *s,int16_t left,int16_t right,
                      uint32_t *output,uint32_t *written){
 uint32_t p=s->w&(RS_H-1u);
 s->l[p]=s->l[p+RS_H]=left;
 s->r[p]=s->r[p+RS_H]=right;
 s->w++;
 p=(s->w-UAC_FIR_N)&(RS_H-1u);
 do {
  const int16_t *x=&s->l[p];
  int64_t al=0,ar=0;
  if(s->ph<UAC_FIR_L/2u){
   const int32_t *c=UAC_FIR[s->ph];
   for(uint32_t j=0u;j<UAC_FIR_N;j++,x++){
    int32_t t=*c++;
    al+=(int64_t)t*x[0];
    ar+=(int64_t)t*x[RS_H*2u];
   }
  }else{
   const int32_t *c=&UAC_FIR[UAC_FIR_L-1u-s->ph][UAC_FIR_N-1u];
   for(uint32_t j=0u;j<UAC_FIR_N;j++,x++){
    int32_t t=*c--;
    al+=(int64_t)t*x[0];
    ar+=(int64_t)t*x[RS_H*2u];
   }
  }
  int16_t yl=(int16_t)clip16((int32_t)((al+(1<<(UAC_FIR_Q-1)))>>UAC_FIR_Q));
  int16_t yr=(int16_t)clip16((int32_t)((ar+(1<<(UAC_FIR_Q-1)))>>UAC_FIR_Q));
  assert(*written<MAX_OUTPUT);
  output[(*written)++]=(uint16_t)yl|((uint32_t)(uint16_t)yr<<16u);
  s->ph+=UAC_FIR_M;
 }while(s->ph<UAC_FIR_L);
 s->ph-=UAC_FIR_L;
}

static void push_block(const int32_t *dma){
 assert(sw-sr<=STAGE_N-HALF);
 for(uint32_t i=0u;i<HALF;i++){
  int32_t l=clip16(dma[i*2u]>>7);
  int32_t r=clip16(dma[i*2u+1u]>>7);
  stage[(sw+i)&(STAGE_N-1u)]=(uint16_t)l|((uint32_t)(uint16_t)r<<16u);
 }
 sw+=HALF;
 if(sw-sr>stage_high)stage_high=sw-sr;
}
static void foreground_one(void){
 assert(sw-sr>=HALF);
 for(uint32_t i=0u;i<HALF;i++){
  transfer[i]=stage[(sr+i)&(STAGE_N-1u)];
 }
 /* Release the SPSC staging queue before the expensive FIR, exactly as
  * firmware does. The now-owned packed transfer cannot be overwritten. */
 sr+=HALF;
 for(uint32_t i=0u;i<HALF;i++){
  uint32_t v=transfer[i];
  int16_t l=(int16_t)(v&0xffffu);
  int16_t r=(int16_t)(v>>16);
  fir_input(&candidate,l,r,candidate_pcm,&nc);
 }
}

int main(void){
 static int32_t dma[HALF*2u];
 for(uint32_t block=0u;block<BLOCKS;block++){
  for(uint32_t i=0u;i<HALF*2u;i++){
   int32_t s=(int16_t)(rnd()>>16);
   if((block&31u)==0u && i<4u)s=(i&1u)?-32768:32767;
   if((block&63u)==1u && i<4u)s=(i&1u)?-1000000:1000000;
   dma[i]=s*128;
  }
  for(uint32_t i=0u;i<HALF;i++)
   fir_input(&reference,(int16_t)clip16(dma[i*2u]>>7),
             (int16_t)clip16(dma[i*2u+1u]>>7),reference_pcm,&nr);
  push_block(dma);
  /* Synthetic LCD pauses of a DMA block: afterwards foreground catches up.
   * Keep the interrupted producer and lower-priority consumer separate. */
  if(block%7u!=0u)foreground_one();
  if(block%7u==1u && sw-sr>=HALF)foreground_one();
 }
 while(sw-sr>=HALF)foreground_one();
 assert(sw==sr);
 assert(stage_high<=STAGE_N);
 assert(nr==nc && nr>70000u);
 assert(memcmp(reference_pcm,candidate_pcm,nr*sizeof(reference_pcm[0]))==0);
 uint64_t hash=UINT64_C(14695981039346656037);
 for(uint32_t i=0u;i<nr;i++){hash^=reference_pcm[i];hash*=UINT64_C(1099511628211);}
 printf("PASS V2.0.30 packed USB 48k FIR PCM exact: input=%u output=%u stage_peak=%u hash=%016llx\n",
        BLOCKS*HALF,nr,stage_high,(unsigned long long)hash);
 return 0;
}
