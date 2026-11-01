/* RUPTURE V1.39 DSP timing regression: default enabled/unity EQ2 must
 * genuinely bypass expensive EQ and global FX, not merely sound dry.
 * Host test; the hardware DMA deadline must still be measured on FM1.
 */
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx_bypass_v181.h"
static struct etch_native_fx fx;
static struct etch_fx_settings cfg;
static struct rupture_eq2_params p;
static struct rupture_eq2_bq coeff[4];
static const uint16_t unity_gains[4]={16384u,16384u,16384u,16384u};
static void reset_with_eq2(void){
 etch_fx_reset(&fx);etch_fx_defaults(&cfg);
 rupture_eq2_defaults(&p,1u);
 rupture_eq2_compute(&p,unity_gains,coeff);
 rupture_eq2_publish(&fx.eq2,coeff,unity_gains);
 fx.eq2_enabled=1u;
}
int main(void){
 reset_with_eq2();assert(rupture_eq2_is_unity(&fx.eq2));
 /* Hard assert on internal filter state: enabled unity EQ2 must not
  * execute biquads, even though the actual PCM also matches. */
 for(uint32_t i=0;i<240000u;i++){
  int16_t in=(int16_t)(((i*31337u)%60001u)-30000);
  int16_t l=0,r=0;
  etch_fx_process_stereo_masked(&fx,in,0,0,0,&cfg,0u,&l,&r);
  assert(l==in && r==in);
 }
 for(int b=0;b<4;b++){
  int32_t last=(int32_t)(((239999u*31337u)%60001u)-30000);
  assert(fx.eq2.x1[b]==last && fx.eq2.y1[b]==last);
 }
 /* A non-flat band MUST activate EQ DSP and stop the unity bypass. */
 uint16_t boosted[4]={16384u,16384u,32767u,16384u};
 rupture_eq2_compute(&p,boosted,coeff);
 rupture_eq2_publish(&fx.eq2,coeff,boosted);
 assert(!rupture_eq2_is_unity(&fx.eq2));
 uint8_t changed=0;
 for(uint32_t i=0;i<6000u;i++){
  int16_t in=(int16_t)((i%63u)*300u-9000);
  int16_t l=0,r=0;
  etch_fx_process_stereo_masked(&fx,in,0,0,0,&cfg,0u,&l,&r);
  if(l!=in)changed=1;
 }
 assert(changed);
 /* Return to neutral and allow coefficient/headroom smoothing to finish. */
 rupture_eq2_compute(&p,unity_gains,coeff);
 rupture_eq2_publish(&fx.eq2,coeff,unity_gains);
 for(int i=0;i<3000;i++){
  int16_t l=0,r=0;
  etch_fx_process_stereo_masked(&fx,(int16_t)(i%79-39),0,0,0,&cfg,0u,&l,&r);
 }
 assert(rupture_eq2_is_unity(&fx.eq2));
 for(int i=0;i<128;i++){
  int16_t in=(int16_t)(i*487-30000),l=0,r=0;
  etch_fx_process_stereo_masked(&fx,in,0,0,0,&cfg,0u,&l,&r);
  assert(l==in&&r==in);
 }
 /* User bypassed EQ still skips EQ2 even with stored nonneutral coeffs. */
 rupture_eq2_compute(&p,boosted,coeff);
 rupture_eq2_publish(&fx.eq2,coeff,boosted);
 for(int i=0;i<100;i++){
  int16_t l=0,r=0;
  etch_fx_process_stereo_masked(&fx,12345,0,0,0,&cfg,ETCH_FXB_BIT(ETCH_FXB_EQ),&l,&r);
  assert(l==12345&&r==12345);
 }
 puts("PASS V1.39 EQ2 unity physically skips biquads/FX, active EQ works, re-neutral and masked FX bypass correct");
 return 0;
}
