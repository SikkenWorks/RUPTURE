/* Channel EQ at minimum controls: PCM regression + EQ-only steady DSP cost.
 * Host timings are relative only; native pi32v2 codegen is audited separately. */
#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"
static struct etch_native_fx fx;
static uint64_t clock_ns(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return (uint64_t)t.tv_sec*1000000000ull+(uint64_t)t.tv_nsec;}
static uint32_t run(uint8_t scenario,uint8_t actual_controls){
 struct etch_fx_settings c;etch_fx_defaults(&c);etch_fx_reset(&fx);
 struct rupture_fx_chain chain;ru_chain_clear(&chain);
 struct rupture_eq2_params p;rupture_eq2_defaults(&p,1u);
 uint16_t g[4]={16384u,16384u,16384u,16384u};
 switch(scenario){
  case 0:g[0]=0u;p.freq[0]=0u;break;
  case 1:g[0]=32767u;p.freq[0]=0u;break;
  case 2:g[1]=0u;p.freq[1]=0u;p.q[0]=100u;break;
  case 3:g[2]=32767u;p.freq[2]=0u;p.q[1]=100u;break;
  case 4:g[0]=32767u;g[1]=0u;g[2]=32767u;g[3]=0u;
   for(unsigned k=0;k<4;k++)p.freq[k]=0u;p.q[0]=p.q[1]=100u;break;
  case 5:break;
  case 6:g[0]=910u;p.freq[0]=0u;break;
  case 7:g[3]=0u;p.freq[3]=0u;break;
 }
 if(actual_controls){
  c.eq_low=g[0];c.eq_mid=g[1];c.eq_high=g[2];
  if(scenario==4u||scenario==7u)c.eq_hpf=32767u;
 }
 struct rupture_eq2_bq coeff[4];rupture_eq2_compute(&p,g,coeff);
 rupture_eq2_publish(&fx.eq2,coeff,g);fx.eq2_enabled=1u;
 uint32_t digest=2166136261u;
 uint64_t start=clock_ns();
 for(uint32_t n=0;n<400000u;n++){
  int16_t input=(int16_t)((((n*17u)^(n>>5u))*17u)%23000u-11500);
  int16_t l=0,r=0;
  rupture_fx_process_ordered(&fx,input,0u,0u,0u,0u,&c,&chain,0u,&l,&r);
  assert(l==r);
  digest=(digest^(uint16_t)l)*16777619u;
 }
 uint64_t elapsed=clock_ns()-start;
 printf("EQ-ONLY case %u saved=%u PCM=%u host %.2f ns/sample\n",scenario,
  actual_controls,digest,(double)elapsed/400000.);
 return digest;
}
static uint32_t reverb_to_eq(void){
 struct etch_fx_settings c;etch_fx_defaults(&c);etch_fx_reset(&fx);
 struct rupture_fx_chain chain;ru_chain_clear(&chain);
 assert(ru_chain_insert(&chain,0u,RU_FX_REVERB));
 struct rupture_eq2_params p;rupture_eq2_defaults(&p,1u);
 p.freq[0]=0u;uint16_t g[4]={0u,16384u,16384u,16384u};
 struct rupture_eq2_bq coeff[4];rupture_eq2_compute(&p,g,coeff);
 rupture_eq2_publish(&fx.eq2,coeff,g);fx.eq2_enabled=1u;
 uint32_t hash=2166136261u;
 for(uint32_t n=0;n<70000u;n++){
  if(n==10000u)chain.enabled=0u;
  int16_t x=(int16_t)(((n*137u)^((n>>6u)*57u))%16000u-8000);
  int16_t l=0,r=0;
  rupture_fx_process_ordered(&fx,x,0u,0u,20000u,0u,&c,&chain,0u,&l,&r);
  hash=(hash^(uint16_t)l)*16777619u;
 }
 printf("REVERB -> EQ low shelf PCM=%u, flush active=%u\n",hash,fx.dry_flush_active);
 return hash;
}
int main(void){
 /* V2.0.9 PCM oracles: never exchange audio quality for DSP headroom. */
 static const uint32_t golden[8]={280933240u,2202590048u,288091765u,
   3391184017u,1215589u,2869647501u,440285323u,4291369326u};
 for(uint8_t c=0;c<8;c++){
  uint32_t oracle=run(c,0u);
  uint32_t hash=run(c,1u);
#ifndef RU_EQ_CAPTURE_ONLY
  if(golden[c])assert(oracle==golden[c]);
  assert(hash==oracle); /* Saved controls may never change the EQ audio. */
#else
  (void)hash;(void)oracle;
#endif
 }
 assert(reverb_to_eq()==3376703322u);
 puts("PASS V2.0.10 EQ-only: 6,400,000 PCM samples, saved knobs equal V2.0.9 EQ transfer");
 return 0;
}
