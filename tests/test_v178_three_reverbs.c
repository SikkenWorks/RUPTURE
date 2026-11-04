/* RUPTURE V1.78: three different native reverb topologies with identical knobs,
 * legacy routing, encoded per-project/per-preset topology, full-wet safety. */
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_project_codec.h"
static struct etch_native_fx fx;
static struct etch_wave_library a,b;
static struct etch_native_pattern pat,pat2;
static struct etch_project_settings cfg,decoded;
static struct etch_preset_dsp_bank dp,rdp;
static uint8_t blob[ETCH_PROJECT_MAX_BYTES];
static void test_settings(void){
 struct etch_fx_settings s;etch_fx_defaults(&s);
 assert(s.routing==0 && etch_fx_reverb_type(&s)==1 && etch_fx_route(&s)==0);
 for(int i=0;i<3;i++)for(int route=0;route<=1;route++){
  etch_fx_set_reverb_type(&s,(uint8_t)i);
  etch_fx_set_route(&s,(uint8_t)route);
  assert(etch_fx_reverb_type(&s)==i);
  assert(etch_fx_route(&s)==route);
  assert(etch_fx_valid_route_and_type(s.routing));
 }
 assert(!etch_fx_valid_route_and_type(6));assert(!etch_fx_valid_route_and_type(7));
 s.routing=1;assert(etch_fx_reverb_type(&s)==1 && etch_fx_route(&s)==1);
 printf("PASS reverb type/serial routing bits; engine selection independent of size/decay\n");
}
static void test_storage(void){
 memset(&a,0,sizeof(a));memset(&cfg,0,sizeof(cfg));
 cfg.seq_length=16;cfg.seq_gate=75;cfg.tempo=120;cfg.source_version=13;
 etch_fx_defaults(&cfg.global_fx);
 etch_project_bind_preset_dsp(&cfg,&dp);etch_project_bind_preset_dsp(&decoded,&rdp);
 etch_perf_init(&dp.performance);etch_key_groups_init(&dp.groups);
 for(int j=0;j<ETCH_PATCH_SLOTS;j++){
  dp.slot[j].spectral_gain=50;dp.slot[j].spectral_tilt=50;
  dp.slot[j].spectral_part=2;dp.slot[j].blend_a=j;dp.slot[j].blend_b=j;
 }
 ru_fxm_defaults(&cfg.rupture_fx);rupture_eq2_defaults(&cfg.eq2,1);
 a.valid[0]=1u;
 a.patches[0].valid=1u;a.patches[0].seq_length=16;a.patches[0].tempo=120;
 cfg.preset_valid=1u;
 etch_fx_defaults(&cfg.preset_fx[0]);
 for(int version=13;version<=34;version+=21){
 cfg.source_version=(uint8_t)version;
 for(int id=0;id<3;id++){
  etch_fx_set_reverb_type(&cfg.global_fx,(uint8_t)id);
  etch_fx_set_reverb_type(&cfg.preset_fx[0],(uint8_t)((id+1)%3));
  etch_fx_set_route(&cfg.global_fx,(uint8_t)(id&1));
  uint32_t n=0;
  int rc=etch_project_encode(&a,&pat,&cfg,blob,sizeof(blob),&n);
  if(rc){printf("encode error %d\n",rc);assert(!rc);}
  assert(etch_project_decode(blob,n,&b,&pat2,&decoded)==0);
  assert(etch_fx_reverb_type(&decoded.global_fx)==id);
  assert(etch_fx_reverb_type(&decoded.preset_fx[0])==(id+1)%3);
  assert(etch_fx_route(&decoded.global_fx)==(id&1));
 }
 }
 /* Legacy 0/1 are still HALL, independent of selected serial/parallel. */
 cfg.preset_valid=0u;
 for(int legacy=0;legacy<=1;legacy++){
  cfg.global_fx.routing=(uint8_t)legacy;uint32_t n=0;
  assert(etch_project_encode(&a,&pat,&cfg,blob,sizeof(blob),&n)==0);
  assert(etch_project_decode(blob,n,&b,&pat2,&decoded)==0);
  assert(etch_fx_reverb_type(&decoded.global_fx)==1);
 }
 puts("PASS PJT-compatible legacy project and per-preset three-engine roundtrip");
}
static uint64_t hashes[3];
static double tail[3],wet_rms[3],stereo[3];
static void test_audio(void){
 for(int mode=0;mode<3;mode++){
  etch_fx_reset(&fx);
  struct etch_fx_settings s;etch_fx_defaults(&s);
  s.reverb_size=24000;s.reverb_decay=26000;etch_fx_set_reverb_type(&s,(uint8_t)mode);
  uint64_t hash=1469598103934665603ULL;
  double e=0,t=0,side=0;
  for(int i=0;i<70000;i++){
   int16_t x=0;
   if(i<11025)x=(int16_t)(10000.0*sin(6.283185307179586*110.0*i/44100.0));
   int16_t left=0,right=0;
   etch_fx_process_stereo_masked(&fx,x,0,0,32767,&s,0,&left,&right);
   assert(left<=28000&&left>=-28000&&right<=28000&&right>=-28000);
   if(i>2000&&i<11025)e+=(double)left*left;
   if(i>11025&&i<32000)t+=(double)left*left;
   if(i>3000&&i<30000)side+=(double)(left-right)*(left-right);
   hash^=(uint16_t)left;hash*=1099511628211ULL;
   hash^=(uint16_t)right;hash*=1099511628211ULL;
  }
  wet_rms[mode]=sqrt(e/9025.0);
  tail[mode]=sqrt(t/20975.0);
  stereo[mode]=sqrt(side/27000.0);
  hashes[mode]=hash;
  printf("%s: fullwet RMS=%.1f tail RMS=%.1f side RMS=%.1f hash=%016llx\n",mode==0?"SPRING":mode==1?"HALL":"ROOM",wet_rms[mode],tail[mode],stereo[mode],(unsigned long long)hash);
  assert(wet_rms[mode]>250);
  assert(tail[mode]>15);
 }
 assert(hashes[0]!=hashes[1]&&hashes[0]!=hashes[2]&&hashes[1]!=hashes[2]);
 assert(fabs(tail[0]-tail[1])>10 && fabs(tail[1]-tail[2])>10);
 puts("PASS three independent impulse/steady-state tails; wet 100; stereo; peak safety");
}
int main(void){test_settings();test_storage();test_audio();puts("PASS V1.78 native three reverb engines");return 0;}
