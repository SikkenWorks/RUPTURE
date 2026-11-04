/* Pure host test: sample-exact 2-bar count-in; cancellation; tempo changes. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_countin.h"
static void run(uint8_t bpm){
 volatile struct RuCountIn c={0};struct RuMetroClick click={0};
 ru_countin_begin(&c);assert(c.active && !c.ready && !c.force_seq && c.beats==0);
 const uint32_t interval=44100u*60u/bpm;
 for(uint32_t sample=0;sample<=interval*8u;sample++){
  uint8_t kick=ru_countin_sample(&c,&click,bpm);
  if(sample<interval*8u){
   assert(!kick);assert(c.active);assert(!c.force_seq);assert(!c.ready);
   assert(c.beats==(uint8_t)(sample/interval+1u));
   if(sample%interval==0){assert(click.sounding && click.pos==0);assert(click.accent==(uint8_t)((sample/interval)%4u==0u));}
  }else{assert(kick && c.ready && c.force_seq && !c.active && c.beats==8);}
 }
 assert(!ru_countin_sample(&c,&click,bpm));
 assert(c.force_seq); /* Holds exact launch while UI publishes seq_run. */
 ru_countin_cancel(&c);assert(!c.force_seq && !c.active && !c.ready);
}
int main(void){
 run(30u);run(60u);run(120u);run(240u);
 volatile struct RuCountIn c={0};struct RuMetroClick click={0};
 ru_countin_begin(&c);assert(!ru_countin_sample(&c,&click,120u));
 assert(c.beats==1u);ru_countin_cancel(&c);
 for(int i=0;i<100000;i++)assert(!ru_countin_sample(&c,&click,120u));
 assert(c.beats==0u&&!c.force_seq);
 /* A tempo change is applied only when the NEXT quarter note begins. */
 ru_countin_begin(&c);
 assert(!ru_countin_sample(&c,&click,120u));
 for(int i=1;i<22050;i++)assert(!ru_countin_sample(&c,&click,200u));
 assert(c.beats==1u);
 assert(!ru_countin_sample(&c,&click,200u));
 assert(c.beats==2u && c.to_next==(44100u*60u/200u)-1u);
 printf("PASS V1.75: sample-exact 8-click two-bar count-in at 30/60/120/240 BPM, accents, cancel, tempo changes, sample-exact step-1 handoff\n");
 return 0;
}
