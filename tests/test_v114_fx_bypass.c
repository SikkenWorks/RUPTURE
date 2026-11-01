#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx_bypass_v181.h"
static struct etch_native_fx st;
static void ensure(int ok,const char *label){if(!ok){fprintf(stderr,"FAIL %s\n",label);__builtin_trap();}}
int main(void){
  struct etch_fx_settings cfg; etch_fx_defaults(&cfg);
  etch_fx_reset(&st);
  /* Exact dry bypass must return unchanged source samples. */
  for(int i=0;i<1000;i++){
    int16_t x=(int16_t)((i*73)%19001-9500),a=0,b=0;
    etch_fx_process_stereo_masked(&st,x,0,0,0,&cfg,ETCH_FXB_ALL,&a,&b);
    ensure(a==x && b==x,"dry parity");
  }
  for(int effect=0;effect<3;effect++){
    etch_fx_reset(&st);
    uint16_t c=effect==0?26000u:0u,d=effect==1?26000u:0u,r=effect==2?26000u:0u;
    uint32_t flag=ETCH_FXB_BIT(effect==0?ETCH_FXB_CHORUS:effect==1?ETCH_FXB_DELAY:ETCH_FXB_REVERB);
    int16_t l=0,rr=0;
    for(int i=0;i<22000;i++) etch_fx_process_stereo_masked(&st,(int16_t)(i%503==0?9000:3000),c,d,r,&cfg,0,&l,&rr);
    int32_t before=effect==0?st.chorus_mix_smooth:effect==1?st.delay_mix_smooth:st.reverb_mix_smooth;
    ensure(before>25000,"wet active");
    etch_fx_process_stereo_masked(&st,3000,c,d,r,&cfg,flag,&l,&rr);
    int32_t after=effect==0?st.chorus_mix_smooth:effect==1?st.delay_mix_smooth:st.reverb_mix_smooth;
    ensure(after>0 && after<before,"first sample fade rather than abrupt cutoff");
    for(int i=0;i<4000;i++) etch_fx_process_stereo_masked(&st,3000,c,d,r,&cfg,flag,&l,&rr);
    after=effect==0?st.chorus_mix_smooth:effect==1?st.delay_mix_smooth:st.reverb_mix_smooth;
    ensure(after==0,"fade completes");
    ensure(l==3000 && rr==3000,"stable bypass returns dry");
  }
  puts("PASS FX bypass: zero-allocation fades, exact dry, no abrupt cutoff");
  return 0;
}
