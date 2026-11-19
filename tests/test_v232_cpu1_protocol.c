#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "rupture_cpu1_protocol.h"

static ru_cpu1_mailbox_t box;
static void prepare(uint32_t seq,uint32_t seed)
{
 memset((void *)&box,0,sizeof box);
 box.seq=seq;box.seed=seed;
 box.check=ru_cpu1_descriptor_check(seq,seed);
 for(uint32_t i=0u;i<RU_C1_WORDS;i++)
  box.input[i]=ru_cpu1_expected_input(seed,i);
}
static void finish(uint32_t seq)
{
 assert(ru_cpu1_process(&box,seq));
 assert(!ru_cpu1_verify(&box,seq)); /* premature acceptance is forbidden */
 box.done=seq;
 assert(ru_cpu1_verify(&box,seq));
}
int main(void)
{
 uint32_t checks=0;
 for(uint32_t j=1u;j<=1000u;j++){
  prepare(j,ru_cpu1_mix(j*0x9e3779b9u));finish(j);checks++;
 }
 for(uint32_t i=0u;i<RU_C1_WORDS;i++)for(uint32_t bit=0u;bit<32u;bit++){
  prepare(3u,0x91234567u);box.input[i]^=(1u<<bit);
  assert(!ru_cpu1_process(&box,3u));checks++;
  prepare(3u,0x91234567u);finish(3u);box.output[i]^=(1u<<bit);
  assert(!ru_cpu1_verify(&box,3u));checks++;
 }
 for(uint32_t bit=0u;bit<32u;bit++){
  prepare(4u,42u);box.check^=(1u<<bit);
  assert(!ru_cpu1_process(&box,4u));checks++;
  prepare(4u,42u);finish(4u);box.result^=(1u<<bit);
  assert(!ru_cpu1_verify(&box,4u));checks++;
 }
 prepare(7u,71u);assert(!ru_cpu1_process(&box,8u));checks++;
 prepare(7u,71u);finish(7u);box.bad=1u;
 assert(!ru_cpu1_verify(&box,7u));checks++;
 printf("PASS V2.0.32 CPU1 mailbox: %u valid/corrupted job cases, all rejected when invalid\n",checks);
 return 0;
}
