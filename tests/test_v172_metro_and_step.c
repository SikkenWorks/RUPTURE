/* V1.72: record metronome must be an independently mixed, beat-aligned
 * voice, unaffected by mono/legato or empty sequencer steps. All patterns
 * operate in RAM; no flash / OTA writes. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_performance.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_seq_entry.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_metronome.h"
int main(void){
 static struct etch_native_pattern p;
 assert(etch_seq_put(&p,0u,53u,100u,4u));
 assert(etch_seq_put(&p,0u,57u,100u,2u));
 assert(etch_seq_put(&p,7u,53u,100u,1u));
 assert(etch_seq_hold_link(&p,16u,1u,53u,0u)==1u);
 assert(etch_seq_hold_link(&p,16u,2u,53u,0u)==1u);
 assert(etch_seq_hold_link(&p,16u,8u,53u,7u)==1u);
 assert(etch_seq_hold_toggle(&p,4u,53u)==1u);
 p.steps[0].lock_mask=1u;p.steps[0].locks[0]=999u;
 p.steps[0].hold_count=1u;p.steps[0].holds[0]=71u;p.steps[0].hold_ago[0]=0u;
 assert(ru_seq_clear_step(&p,0u)==1u);
 assert(!p.steps[0].count&&!p.steps[0].hold_count&&!p.steps[0].lock_mask&&!p.steps[0].locks[0]);
 assert(!p.steps[1].hold_count&&!p.steps[2].hold_count);
 assert(p.steps[7].count==1u&&p.steps[8].hold_count==1u);
 assert(p.steps[4].hold_count==1u && p.steps[4].hold_ago[0]==0u);
 assert(ru_seq_clear_step(&p,0u)==0u);
 assert(ru_seq_key_toggle(&p,0u,55u,87u)==1u); /* can record again */
 static struct etch_native_pattern snapshot;
 p.steps[3].hold_count=9u;snapshot=p;
 assert(!ru_seq_clear_step(&p,0u));assert(!memcmp(&p,&snapshot,sizeof p));
 struct RuMetroClick m={0};
 unsigned clicks=0,beat_steps=0,peak=0;
 for(unsigned step=0;step<32u;step++){
  ru_metronome_step(&m,(uint16_t)step,2u,1u); /* 16th note step rate, quarter click */
  if(m.sounding){beat_steps++;if(step%4u==0u)clicks++;}
  unsigned audible=0;
  for(unsigned n=0;n<256u;n++){
   int16_t v=ru_metronome_sample(&m,1u,100u);
   if(v){audible++;if((unsigned)(v<0?-v:v)>peak)peak=(unsigned)(v<0?-v:v);}
  }
  if(step%4u==0u)assert(audible>20u);
  else assert(audible==0u);
 }
 assert(clicks==8u && beat_steps==8u && peak>100u);
 ru_metronome_step(&m,0u,2u,1u);
 assert(ru_metronome_sample(&m,0u,100u)==0);assert(!m.sounding);
 ru_metronome_step(&m,0u,2u,0u);assert(!m.sounding);
 ru_metronome_step(&m,4u,2u,1u);assert(ru_metronome_sample(&m,1u,0u)==0&&!m.sounding);
 puts("PASS V1.72: complete step+HOLD+lock clearing, corruption atomicity, 8 beat clicks, REC off/volume zero silence");
 return 0;
}
