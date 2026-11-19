/* V2.0.26: exhaustive DISTINCT FX-subset sweep (up to eight slots) plus
 * repeats, sparse racks and every ordered pair. Not a physical FM1 profiler.
 * Build once with -DRU_FX_SAND_CACHE=0 and once with =1; compare PCM hashes. */
#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_sub_realtime.h"

static struct etch_native_fx fx_state;
static uint64_t hash=UINT64_C(14695981039346656037);
static uint64_t cases=0u,frames=0u;
static uint64_t total_ns[RU_FX_TYPES],total_samples[RU_FX_TYPES];
static uint64_t reverb_pair_ns[RU_FX_TYPES],reverb_pair_samples[RU_FX_TYPES];
static uint64_t permutations=0u;
static uint64_t sub1_frames=0u;
static const char *names[RU_FX_TYPES]={"REVERB","DELAY","CHORUS","MOSFET","COMP","TILT","FUZZ","SANDPAPER","DISTORTION","WAH"};

static uint64_t ns_now(void){
 struct timespec t;
 assert(clock_gettime(CLOCK_MONOTONIC,&t)==0);
 return (uint64_t)t.tv_sec*1000000000ull+(uint64_t)t.tv_nsec;
}
static void run(const uint8_t *kinds,uint8_t count,uint8_t reverse,
                uint8_t independent,uint8_t reverb_mode,uint8_t sparse,
                uint32_t nframes,uint8_t benchmark){
 struct rupture_fx_chain chain;
 struct etch_fx_settings settings;
 ru_chain_clear(&chain);etch_fx_defaults(&settings);
 etch_fx_set_reverb_type(&settings,reverb_mode);
 settings.reverb_decay=26500u;settings.reverb_size=29500u;
 settings.delay_feedback=23500u;
 settings.wah_mix=29000u;settings.folder_mix=27000u;
 settings.comp_ratio=26500u;
 chain.independent=independent;
 for(uint8_t i=0u;i<count;i++){
  uint8_t kind=kinds[reverse?(uint8_t)(count-1u-i):i];
  uint8_t slot=sparse?(uint8_t)(RU_FX_SLOTS-count+i):i;
  assert(ru_chain_insert(&chain,slot,kind));
  if(kind==RU_FX_REVERB)chain.parameter[slot][0]=reverb_mode;
  if(kind==RU_FX_SAND){
   chain.parameter[slot][0]=85u;chain.parameter[slot][1]=89u;
   chain.parameter[slot][2]=77u;chain.parameter[slot][3]=96u;
  }
  if(kind==RU_FX_WAH)chain.parameter[slot][3]=30000u;
  if(kind==RU_FX_DISTORTION)chain.parameter[slot][3]=30000u;
  if(kind==RU_FX_FUZZ)chain.parameter[slot][3]=95u;
 }
 etch_fx_reset(&fx_state);
 struct rupture_fx_dispatch dispatch=ru_chain_prepare_dispatch(&chain);
 struct rupture_fx_distortion_cache distortion_cache={0};
 uint8_t checked=0u;
 uint32_t rng=0x9e3779b9u^(uint32_t)(count*217u+reverb_mode*41u+independent*11u+sparse*103u);
 uint64_t start=ns_now();
 for(uint32_t i=0u;i<nframes;i++){
  if(!(i&31u)){checked=0u;distortion_cache.valid_mask=0u;}
  /* Exercise sample-exact wet ramps and changing saved Sandpaper MIX. */
  if(nframes==512u && i==128u){
   for(uint8_t k=0u;k<RU_FX_SLOTS;k++)if(chain.type[k]==RU_FX_SAND)
    chain.parameter[k][3]=23u;
   chain.enabled^=0xaau;
   for(uint8_t k=0u;k<RU_FX_SLOTS;k++)if(chain.type[k]==RU_FX_EMPTY)
    chain.enabled&=(uint8_t)~(1u<<k);
  }
  if(nframes==512u && i==256u){
   for(uint8_t k=0u;k<RU_FX_SLOTS;k++)if(chain.type[k]==RU_FX_SAND)
    chain.parameter[k][3]=99u;
   chain.enabled=0xffu;
   for(uint8_t k=0u;k<RU_FX_SLOTS;k++)if(chain.type[k]==RU_FX_EMPTY)
    chain.enabled&=(uint8_t)~(1u<<k);
  }
  rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;
  int16_t input=(int16_t)((rng&0xffffu)-32768u);
  int16_t l=0,r=0;
  rupture_fx_process_ordered_fast(&fx_state,input,24000u,22000u,26000u,19000u,
   &settings,&chain,0u,&l,&r,&dispatch,&checked,&distortion_cache);
  /* Every output frame contributes to this deterministic full-matrix hash. */
  uint32_t word=(uint16_t)l|((uint32_t)(uint16_t)r<<16u);
  hash^=word;hash*=UINT64_C(1099511628211);
 }
 uint64_t elapsed=ns_now()-start;
 if(benchmark&&count==1u){total_ns[kinds[0]]+=elapsed;total_samples[kinds[0]]+=nframes;}
 if(nframes==512u&&count==2u){
  uint8_t other=kinds[0]==RU_FX_REVERB?kinds[1]:kinds[1]==RU_FX_REVERB?kinds[0]:RU_FX_EMPTY;
  if(other!=RU_FX_EMPTY){reverb_pair_ns[other]+=elapsed;reverb_pair_samples[other]+=nframes;}
 }
 cases++;frames+=nframes;
}
static void run_sub1_pair(uint8_t kind,uint8_t reverb_mode,
                          uint8_t reverse,uint8_t independent){
 struct rupture_fx_chain chain;
 struct etch_fx_settings settings;
 ru_chain_clear(&chain);etch_fx_defaults(&settings);
 etch_fx_set_reverb_type(&settings,reverb_mode);
 settings.reverb_decay=29000u;settings.reverb_size=30000u;
 chain.independent=independent;
 uint8_t kinds[2]={RU_FX_REVERB,kind};
 for(uint8_t i=0u;i<2u;i++){
  uint8_t type=kinds[reverse?1u-i:i];
  assert(ru_chain_insert(&chain,i,type));
  if(type==RU_FX_REVERB)chain.parameter[i][0]=reverb_mode;
  if(type==RU_FX_SAND){
   chain.parameter[i][0]=85u;chain.parameter[i][1]=89u;
   chain.parameter[i][2]=77u;chain.parameter[i][3]=96u;
  }
 }
 etch_fx_reset(&fx_state);
 RuSubState sub;
 RuSubRt rt={0};
 ru_sub_init(&sub,0x013579bdu,0xcade4f34u);
 struct rupture_fx_dispatch dispatch=ru_chain_prepare_dispatch(&chain);
 struct rupture_fx_distortion_cache distortion_cache={0};
 uint8_t checked=0u;
 for(uint32_t frame=0u;frame<4096u;frame++){
  if(!(frame&31u)){checked=0u;distortion_cache.valid_mask=0u;}
  int32_t bass=ru_sub_rt_tick(&rt,&sub,RU_SUB_CXS,1u,52u,25u,
     0x013579bdu,68u,57u,62u,66u);
  int32_t input=(frame&1u?-13500:13500)+(bass>>2u);
  if(input>32767)input=32767;
  if(input< -32768)input=-32768;
  int16_t l,r;
  rupture_fx_process_ordered_fast(&fx_state,(int16_t)input,22000u,21000u,28000u,18000u,
   &settings,&chain,0u,&l,&r,&dispatch,&checked,&distortion_cache);
  hash^=(uint16_t)l|((uint32_t)(uint16_t)r<<16u);
  hash*=UINT64_C(1099511628211);
  sub1_frames++;
 }
 cases++;frames+=4096u;
}
int main(void){
 rupture_tilt_controls(73u,71u,87u,81u,1u);
 rupture_fuzz_controls(85u,83u,80u,95u,1u);
 rupture_sandpaper_controls(85u,89u,77u,96u,1u);
 /* All 1,013 possible unique-effect subsets of one through eight out of ten.
  * Two directions, three reverb engines, independent and legacy parameters. */
 for(uint16_t mask=1u;mask<1024u;mask++){
  uint8_t kinds[RU_FX_TYPES],count=0u;
  for(uint8_t k=0u;k<RU_FX_TYPES;k++)if(mask&(1u<<k))kinds[count++]=k;
  if(count>RU_FX_SLOTS)continue;
  for(uint8_t independent=0u;independent<2u;independent++)
   for(uint8_t engine=0u;engine<3u;engine++)
    for(uint8_t reverse=0u;reverse<2u;reverse++)
     run(kinds,count,reverse,independent,engine,reverse&&count<8u,128u,0u);
 }
 /* Every distinct ordering of one, two, three and four inserts: this
  * includes all 5,860 short-chain permutations (times six control modes). */
 for(uint8_t count=1u;count<=4u;count++){
  uint32_t limit=1u;
  for(uint8_t i=0u;i<count;i++)limit*=10u;
  for(uint32_t encoded=0u;encoded<limit;encoded++){
   uint8_t kinds[4],valid=1u;
   uint16_t seen=0u;
   uint32_t v=encoded;
   for(uint8_t i=0u;i<count;i++){
    kinds[i]=(uint8_t)(v%10u);v/=10u;
    if(seen&(1u<<kinds[i])){valid=0u;break;}
    seen|=(uint16_t)(1u<<kinds[i]);
   }
   if(!valid)continue;
   permutations++;
   for(uint8_t independent=0u;independent<2u;independent++)
    for(uint8_t engine=0u;engine<3u;engine++)
     run(kinds,count,0u,independent,engine,0u,128u,0u);
  }
 }
 assert(permutations==5860u);
 /* Every ordered pair, including duplicated FX, long after wet fades settle. */
 for(uint8_t a=0u;a<RU_FX_TYPES;a++)
  for(uint8_t b=0u;b<RU_FX_TYPES;b++){
   uint8_t kinds[]={a,b};
   for(uint8_t independent=0u;independent<2u;independent++)
    for(uint8_t engine=0u;engine<3u;engine++)
     run(kinds,2u,0u,independent,engine,0u,512u,0u);
  }
 /* Repeated and maximum-density racks, including extra SAND and REVERB. */
 for(uint8_t kind=0u;kind<RU_FX_TYPES;kind++){
  uint8_t all[8];for(uint8_t i=0u;i<8u;i++)all[i]=kind;
  for(uint8_t engine=0u;engine<3u;engine++)
   run(all,8u,0u,1u,engine,0u,768u,0u);
 }
 for(uint8_t kind=0u;kind<RU_FX_TYPES;kind++){
  uint8_t one[]={kind};
  run(one,1u,0u,1u,2u,0u,65536u,1u);
 }
 /* Real quarter-rate SUB1 CXS into REVERB plus every other effect, in
  * both orders, both parameter modes and all three reverb topologies. */
 for(uint8_t kind=0u;kind<RU_FX_TYPES;kind++)
  for(uint8_t engine=0u;engine<3u;engine++)
   for(uint8_t reverse=0u;reverse<2u;reverse++)
    for(uint8_t independent=0u;independent<2u;independent++)
     run_sub1_pair(kind,engine,reverse,independent);
 printf("FX_MATRIX configs=%llu ordered_short_chains=%llu stereo_frames=%llu real_sub1_frames=%llu hash=%016llx\n",
  (unsigned long long)cases,(unsigned long long)permutations,
  (unsigned long long)frames,(unsigned long long)sub1_frames,(unsigned long long)hash);
 for(uint8_t k=0u;k<RU_FX_TYPES;k++)printf("FX_COST %-11s ns_per_frame=%.1f\n",names[k],
  (double)total_ns[k]/(double)total_samples[k]);
 for(uint8_t k=0u;k<RU_FX_TYPES;k++)printf("FX_REVERB_PAIR %-11s ns_per_frame=%.1f\n",names[k],
  (double)reverb_pair_ns[k]/(double)reverb_pair_samples[k]);
 return 0;
}
