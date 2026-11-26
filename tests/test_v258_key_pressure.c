/* V2.0.58 host behavioral regression: expression never emits note events.
 * Actual GPIO timing remains hardware-unverified until the FM1 test. */
#define RUPTURE_BUILD 1
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_key_touch.h"

int main(void){
 struct etch_native_controls c={0},original={0};
 struct ru_touch_state touch={.active_key=255u};
 uint16_t readings[20]={0},generations[20]={0};
 c.cutoff=5000u;c.cxs_pressure=60u;c.sub_blend[0]=0u;
 c.fx_chain_active=1u;c.fx_chain.independent=1u;
 ru_chain_clear(&c.fx_chain);
 c.fx_chain.type[0]=RU_FX_MOSFET;
 c.fx_chain.parameter[0][0]=6000u;c.fx_chain.enabled=1u;
 original=c;
 ru_touch_apply(&c,ru_touch_config(RU_TOUCH_FILTER,1u),0u);
 assert(!memcmp(&c,&original,sizeof(c)));
 ru_touch_apply(&c,ru_touch_config(RU_TOUCH_OFF,1u),127u);
 assert(!memcmp(&c,&original,sizeof(c)));

 const uint8_t cfg=ru_touch_config(RU_TOUCH_FILTER,1u);
 assert(ru_touch_update(&touch,1u,readings,generations,20u,cfg)==0u);
 readings[0]=420u;generations[0]=1u;
 assert(ru_touch_update(&touch,1u,readings,generations,40u,cfg)==0u);
 readings[0]=300u;generations[0]=2u;
 assert(ru_touch_update(&touch,1u,readings,generations,60u,cfg)>0u);
 assert(touch.raw==300u);
 assert(touch.pressure<=127u);
 /* Relaxing the finger recentres the light-contact reference. A later
  * press must respond again, without retriggering a digital note. */
 uint8_t before_release=touch.pressure;
 readings[0]=435u;generations[0]=3u;
 assert(ru_touch_update(&touch,1u,readings,generations,80u,cfg)<before_release);
 assert(touch.landing==435u);
 readings[0]=315u;generations[0]=4u;
 assert(ru_touch_update(&touch,1u,readings,generations,100u,cfg)>0u);
 ru_touch_apply(&c,cfg,touch.pressure);
 assert(c.cutoff>original.cutoff);
 assert(c.cxs_pressure==original.cxs_pressure);
 assert(c.fx_chain.parameter[0][0]==original.fx_chain.parameter[0][0]);
 assert(ru_touch_update(&touch,0u,readings,generations,120u,cfg)==0u);
 assert(touch.raw==0u);

 /* The seven PA6 keys never inherit analog data from the last held note. */
 assert(ru_touch_update(&touch,1u<<26,readings,generations,140u,cfg)==0u);
 assert(ru_touch_update(&touch,1u<<26,readings,generations,160u,cfg)==0u);
 assert(touch.raw==0u);

 c=original;
 ru_touch_apply(&c,ru_touch_config(RU_TOUCH_GROWL,2u),127u);
 assert(!memcmp(&c,&original,sizeof(c))); /* CXS mode: bit identical */
 c.main_wave=1u;
 ru_touch_apply(&c,ru_touch_config(RU_TOUCH_GROWL,2u),127u);
 assert(c.cxs_pressure>original.cxs_pressure);

 c=original;
 ru_touch_apply(&c,ru_touch_config(RU_TOUCH_MOSFET,0u),127u);
 assert(c.fx_chain.parameter[0][0]>original.fx_chain.parameter[0][0]);
 assert(c.cutoff==original.cutoff);
 c=original;
 ru_touch_apply(&c,ru_touch_config(RU_TOUCH_SUB1,0u),127u);
 assert(!memcmp(&c,&original,sizeof(c))); /* a disabled sub stays disabled */

 printf("PASS V2.0.58 pressure target, smoothing, release, top 7 neutral, OFF/CXS exact\n");
 return 0;
}
