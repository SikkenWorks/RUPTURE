#include <stdio.h>
#include <stdint.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx_bypass_v181.h"
static struct etch_native_fx s;
static int64_t run(uint16_t chorus,uint16_t delay,uint16_t reverb,uint32_t mask,struct etch_fx_settings *cfg){
 etch_fx_reset(&s);int64_t hash=0;
 for(int i=0;i<20000;i++){
  int16_t l=0,r=0,vin=(int16_t)(i==0?20000:(i%101==0?6000:0));
  etch_fx_process_stereo_masked(&s,vin,chorus,delay,reverb,cfg,mask,&l,&r);
  hash=hash*1099511628211ULL+(uint16_t)l+(uint16_t)r*17u;
 }
 return hash;
}
int main(void){
 struct etch_fx_settings cfg;etch_fx_defaults(&cfg);
 for(int i=0;i<3;i++){
  uint16_t c=i==0?11000:0,d=i==1?11000:0,r=i==2?11000:0;
  int64_t on=run(c,d,r,0,&cfg);
  int64_t off=run(c,d,r,ETCH_FXB_BIT(i==0?ETCH_FXB_CHORUS:i==1?ETCH_FXB_DELAY:ETCH_FXB_REVERB),&cfg);
  if(on==off){printf("FX %d indistinguishable on/bypass\n",i);return 1;}
 }
 cfg.eq_low=22000;cfg.eq_mid=12000;cfg.eq_high=19000;cfg.eq_hpf=7000;
 int64_t eq=run(0,0,0,0,&cfg),flat=run(0,0,0,ETCH_FXB_BIT(ETCH_FXB_EQ),&cfg);
 if(eq==flat)return 2;
 puts("PASS: chorus, delay, reverb, EQ produce distinct PCM versus bypass");return 0;
}
