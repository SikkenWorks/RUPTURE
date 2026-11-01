/* RUPTURE PJT33: FX persistence, old project defaults and corruption detection. */
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_project_codec.h"
static struct etch_wave_library a,b;
static struct etch_native_pattern p,q;
static struct etch_project_settings cfg,got;
static struct etch_preset_dsp_bank bank,loadbank;
static uint8_t archive[ETCH_PROJECT_MAX_BYTES];
/* Regression guard: all project commit paths MUST keep the PJT33 marker. */
static void assert_capture_version(void){
 FILE *f=fopen("vendor/etch-v1.88-source/firmware/fm1-native/wave_store_target.c","rb");
 assert(f);char buf[12000];size_t n=fread(buf,1,sizeof(buf)-1,f);fclose(f);buf[n]=0;
 assert(strstr(buf,"native_project_cfg.source_version=34;")!=NULL);
}
static void setup(void){
 memset(&a,0,sizeof(a));memset(&b,0,sizeof(b));
 memset(&p,0,sizeof(p));memset(&q,0,sizeof(q));
 memset(&cfg,0,sizeof(cfg));memset(&got,0,sizeof(got));
 memset(&bank,0,sizeof(bank));memset(&loadbank,0,sizeof(loadbank));
 cfg.seq_length=16;cfg.tempo=120;cfg.seq_gate=75;cfg.source_version=33;
 etch_fx_defaults(&cfg.global_fx);etch_project_bind_preset_dsp(&cfg,&bank);
 etch_project_bind_preset_dsp(&got,&loadbank);
 a.valid[0]=1u;
 for(uint8_t i=0;i<ETCH_PATCH_SLOTS;i++){
  bank.slot[i].spectral_gain=50;bank.slot[i].spectral_tilt=50;
  bank.slot[i].spectral_part=2;bank.slot[i].blend_a=i;bank.slot[i].blend_b=i;
 }
 etch_perf_init(&bank.performance);etch_key_groups_init(&bank.groups);
 ru_fxm_defaults(&cfg.rupture_fx);
}
static uint32_t save(void){uint32_t used=0;
 assert(etch_project_encode(&a,&p,&cfg,archive,sizeof(archive),&used)==0);return used;}
static void check(const struct rupture_fx_memory *x){
 assert(memcmp(&got.rupture_fx,x,sizeof(*x))==0);
 assert(ru_fxm_valid(&got.rupture_fx));
}
int main(void){
 assert_capture_version();setup();struct rupture_fx_memory fresh=cfg.rupture_fx;
 assert(!fresh.enabled&&!fresh.bypass);
 uint32_t n=save();assert(n>21);
 assert(archive[0]=='P'&&archive[1]=='J'&&archive[2]=='T'&&archive[3]=='Q');
 assert(etch_project_decode(archive,n,&b,&q,&got)==0);
 assert(got.source_version==33);check(&fresh);
 for(unsigned e=0;e<4;e++)for(unsigned k=0;k<4;k++)cfg.rupture_fx.knobs[e][k]=(uint8_t)((e*23+k*19)%101);
 cfg.rupture_fx.enabled=0x0d;
 cfg.rupture_fx.bypass=(1u<<0)|(1u<<3)|(1u<<6)|(1u<<11)|(1u<<21);
 n=save();assert(etch_project_decode(archive,n,&b,&q,&got)==0);
 struct rupture_fx_memory expect=cfg.rupture_fx;check(&expect);
 assert(got.tempo==120 && got.seq_length==16);
 /* Project A then B: no leakage even when same output object reused. */
 cfg.rupture_fx=fresh;n=save();
 assert(etch_project_decode(archive,n,&b,&q,&got)==0);check(&fresh);
 /* Old versions never keep stale values from previously-loaded projects. */
 cfg.source_version=32;
 n=save();assert(etch_project_decode(archive,n,&b,&q,&got)==0);
 assert(got.source_version==32);check(&fresh);
 /* PJT33 appended bytes must be validated, not silently clamped. */
 cfg.source_version=33;cfg.rupture_fx=expect;n=save();
 archive[n-1]|=0x80u;assert(etch_project_decode(archive,n,&b,&q,&got)!=0);
 n=save();archive[n-5]=0x80u;assert(etch_project_decode(archive,n,&b,&q,&got)!=0);
 n=save();archive[n-21]=101u;assert(etch_project_decode(archive,n,&b,&q,&got)!=0);
 n=save();assert(etch_project_decode(archive,n-1,&b,&q,&got)!=0);
 /* Encoder refuses invalid memory before touching destination bytes. */
 cfg.rupture_fx.knobs[2][1]=103;
 assert(etch_project_encode(&a,&p,&cfg,archive,sizeof(archive),&n)==-1);
 puts("PASS PJT33 FX memory: all 16 parameters, 4 enabled, 22 bypass bits; old-project reset, cross-project isolation, corrupt/truncated reject");
 return 0;
}
