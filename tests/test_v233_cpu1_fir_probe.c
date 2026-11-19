#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "uac_fir.h"
#include "rupture_cpu1_fir_probe.h"

static ru_cpu1_mailbox_t box;
static uint32_t tests;

static void prepare(uint32_t seq,uint32_t seed)
{
 memset((void *)&box,0,sizeof box);
 box.seq=seq;
 box.seed=seed;
 box.kind=RU_C1_JOB_FIR;
 box.check=ru_cpu1_fir_check(seq,seed);
 for(uint32_t i=0u;i<RU_C1_WORDS;i++)
  box.input[i]=ru_cpu1_expected_input(seed,i);
}

static void completed(uint32_t seq)
{
 assert(ru_cpu1_fir_process(&box,seq));
 assert(!ru_cpu1_fir_verify(&box,seq)); /* no premature result commit */
 box.done=seq;
 assert(ru_cpu1_fir_verify(&box,seq));
 tests++;
}

int main(void)
{
 for(uint32_t j=1u;j<=1000u;j++){
  prepare(j,ru_cpu1_mix(j*0x9e3779b9u));completed(j);
  assert(!ru_cpu1_verify(&box,j)); /* FIR result cannot masquerade as RAM job */
 }
 for(uint32_t i=0u;i<RU_C1_WORDS;i++)for(uint32_t b=0u;b<32u;b++){
  prepare(7u,0x8acb4102u);box.input[i]^=1u<<b;
  assert(!ru_cpu1_fir_process(&box,7u));tests++;
  prepare(7u,0x8acb4102u);completed(7u);
  box.output[i]^=1u<<b;
  assert(!ru_cpu1_fir_verify(&box,7u));tests++;
 }
 for(uint32_t b=0u;b<32u;b++){
  prepare(9u,0x8912387au);box.check^=1u<<b;
  assert(!ru_cpu1_fir_process(&box,9u));tests++;
  prepare(9u,0x8912387au);completed(9u);
  box.result^=1u<<b;
  assert(!ru_cpu1_fir_verify(&box,9u));tests++;
 }
 prepare(19u,0xfefad07eu);box.kind=RU_C1_JOB_INTEGRITY;
 assert(!ru_cpu1_fir_process(&box,19u));tests++;
 prepare(19u,0xfefad07eu);box.done=18u;
 assert(!ru_cpu1_fir_verify(&box,19u));tests++;
 prepare(19u,0xfefad07eu);box.check=ru_cpu1_descriptor_check(19u,box.seed);
 assert(!ru_cpu1_fir_process(&box,19u));tests++;
 printf("PASS V2.0.33 CPU1 24-tap FIR: %u valid and injected-corruption cases\n",tests);
 return 0;
}
