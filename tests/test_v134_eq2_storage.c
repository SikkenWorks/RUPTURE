/* RUPTURE PJT34 new EQ2 controls: old projects retain legacy DSP mode;
 * complete 4freq + 2Q survives project save/load; corrupt bytes rejected. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_project_codec.h"
static struct etch_wave_library a,b;
static struct etch_project_settings settings,out;
static struct etch_native_pattern pat,readpat;
static struct etch_preset_dsp_bank dp,rdp;
static uint8_t packet[ETCH_PROJECT_MAX_BYTES];
static uint32_t save(void){uint32_t n=0;assert(etch_project_encode(&a,&pat,&settings,packet,sizeof(packet),&n)==0);return n;}
int main(void){
 memset(&a,0,sizeof(a));memset(&settings,0,sizeof(settings));
 settings.seq_length=16;settings.seq_gate=75;settings.tempo=120;
 etch_fx_defaults(&settings.global_fx);ru_fxm_defaults(&settings.rupture_fx);
 etch_project_bind_preset_dsp(&settings,&dp);etch_project_bind_preset_dsp(&out,&rdp);
 etch_perf_init(&dp.performance);etch_key_groups_init(&dp.groups);
 for(int i=0;i<ETCH_PATCH_SLOTS;i++){
  dp.slot[i].spectral_gain=50;dp.slot[i].spectral_tilt=50;dp.slot[i].spectral_part=2;
  dp.slot[i].blend_a=i;dp.slot[i].blend_b=i;
 }
 a.valid[0]=1u;
 settings.source_version=33;
 uint32_t n=save();assert(etch_project_decode(packet,n,&b,&readpat,&out)==0);
 assert(out.source_version==33&&!out.eq2.enabled);
 assert(out.eq2.q[0]==9&&out.eq2.q[1]==9);
 rupture_eq2_defaults(&settings.eq2,1);
 settings.eq2.freq[0]=33;settings.eq2.freq[1]=72;settings.eq2.freq[2]=8;settings.eq2.freq[3]=94;
 settings.eq2.q[0]=15;settings.eq2.q[1]=89;
 settings.source_version=34;
 n=save();assert(packet[0]=='P'&&packet[1]=='J'&&packet[2]=='T'&&packet[3]=='R');
 assert(etch_project_decode(packet,n,&b,&readpat,&out)==0);
 assert(out.source_version==34&&out.eq2.enabled);
 assert(memcmp(&settings.eq2,&out.eq2,sizeof(settings.eq2))==0);
 packet[n-2]=255;assert(etch_project_decode(packet,n,&b,&readpat,&out)!=0);
 n=save();packet[n-7]=2;assert(etch_project_decode(packet,n,&b,&readpat,&out)!=0);
 n=save();assert(etch_project_decode(packet,n-1,&b,&readpat,&out)!=0);
 puts("PASS PJT34 7-byte EQ2 save/load, backwards legacy defaults, corrupt/truncated rejection");
}
