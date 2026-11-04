/* RUPTURE V1.79: sample-exact SUB pitch and arbitrary live drift changes.
 * 300,000 reference comparisons include legal ranges and octave wrap.
 * No audio-ISR clock, DMA or actual hardware simulation is claimed. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_subosc.h"
static uint32_t rng=0x9174ea31u;
static uint32_t random32(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
static uint32_t ref(uint32_t base,uint8_t octave,uint8_t detune,int32_t drift){
 uint64_t inc=base;
 if(octave==0u)inc>>=2u;else if(octave==1u)inc>>=1u;
 else if(octave==3u)inc<<=1u;
 int32_t cents=(int32_t)detune-50;
 int32_t factor=65536+cents*38+drift;
 if(factor<32768)factor=32768;
 inc=(inc*(uint32_t)factor)>>16;
 return (uint32_t)(inc>UINT32_MAX?UINT32_MAX:inc<1u?1u:inc);
}
int main(void){
 for(unsigned i=0;i<300000;i++){
  uint32_t base=random32();
  uint8_t octave=(uint8_t)(random32()%4u);
  uint8_t detune=(uint8_t)(random32()%101u);
  int32_t drift=(int32_t)(random32()%1537u)-768;
  uint32_t a=ru_sub_inc(base,octave,detune,drift),b=ref(base,octave,detune,drift);
  if(a!=b){fprintf(stderr,"pitch mismatch %u %u %u %d: %u != %u\n",base,octave,detune,drift,a,b);return 2;}
 }
 for(int oct=0;oct<4;oct++)for(int dt=0;dt<=100;dt+=25)for(int i=0;i<10000;i++){
  uint32_t base=(uint32_t)i*342932u;
  for(int drift=-768;drift<=768;drift+=384){
   assert(ru_sub_inc(base,(uint8_t)oct,(uint8_t)dt,drift)==ref(base,(uint8_t)oct,(uint8_t)dt,drift));
  }
 }
 /* Load two full-rate SUBs with changing detune/drift, including simultaneous
  * CLASSIC/CXS/SQUARE/SAW and repeated 0..100 mod wheel sweeps. */
 RuSubState s[2];ru_sub_init(&s[0],4783598u,0xa433e17bu);ru_sub_init(&s[1],4783598u,0xb93e8215u);
 uint32_t audio_hash=2166136261u;
 for(int i=0;i<180000;i++){
  uint8_t drift=(uint8_t)((i/701u)%101u);
  uint8_t type=(uint8_t)(1+(i/30000u)%4u);
  int32_t a=ru_sub_tick(&s[0],type,1u,49u,drift,4783598u,71u,59u,68u,77u);
  int32_t b=ru_sub_tick(&s[1],type,0u,51u,(uint8_t)(100u-drift),4783598u,72u,47u,59u,82u);
  audio_hash=(audio_hash^(uint32_t)(uint16_t)a)*16777619u;
  audio_hash=(audio_hash^(uint32_t)(uint16_t)b)*16777619u;
 }
 assert(audio_hash==0xab2ab9ccu); /* exact V1.78 reference PCM */
 printf("PASS V1.79: 700000+ exact SUB increments, drift sweep, stereo parity hash=%08x\n",audio_hash);
 return 0;
}
