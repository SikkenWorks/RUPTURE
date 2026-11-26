/* V2.0.58: CXS/GROWL expression ownership and FX insertion without loss.
 * Hardware ISR timing must be qualified on a physical FM1. */
#define RUPTURE_BUILD 1
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_key_touch.h"
int main(void){
 struct ru_touch_profile cxs={RU_TOUCH_FILTER,RU_TOUCH_MODE_PRESS,1u,10u,40u,1u};
 struct ru_touch_profile growl={RU_TOUCH_GROWL,RU_TOUCH_MODE_RANDOM,3u,25u,75u,2u};
 assert(ru_touch_profile_valid(&cxs)&&ru_touch_profile_valid(&growl));
 assert(!ru_touch_profile_equal(&cxs,&growl));
 struct etch_native_controls base={0},c;
 base.cutoff=13337u;base.cxs_pressure=59u;
 ru_chain_clear(&base.fx_chain);base.fx_chain_active=1u;
 c=base;ru_touch_apply_profile(&c,&cxs,0u);
 assert(c.cutoff==ru_touch_percent_q15(10u));
 c=base;ru_touch_apply_profile(&c,&cxs,127u);
 assert(c.cutoff==ru_touch_percent_q15(40u));
 assert(c.cxs_pressure==base.cxs_pressure);
 c=base;ru_touch_apply_profile(&c,&growl,127u);
 assert(!memcmp(&c,&base,sizeof(c))); /* GROWL pressure cannot alter CXS. */
 c.main_wave=1u;ru_touch_apply_profile(&c,&growl,127u);
 assert(c.cxs_pressure==75u);
 c=base;c.main_wave=0u;
 struct ru_touch_profile off=RU_TOUCH_PROFILE_DEFAULT;
 ru_touch_apply_profile(&c,&off,127u);
 assert(c.cutoff==base.cutoff&&c.cxs_pressure==base.cxs_pressure);

 struct ru_touch_drift_state drift={0};
 cxs.mode=RU_TOUCH_MODE_DRIFT;
 assert(ru_touch_motion(&drift,&cxs,500u,1u<<26,0u)==0u);
 assert(ru_touch_motion(&drift,&cxs,2500u,1u<<26,0u)>0u);
 assert(ru_touch_motion(&drift,&cxs,2600u,0u,0u)==0u);
 cxs.mode=RU_TOUCH_MODE_RANDOM;
 assert(ru_touch_motion(&drift,&cxs,3000u,1u,0u)==0u);
 assert(ru_touch_motion(&drift,&cxs,4300u,1u,0u)>0u);
 assert(ru_touch_motion(&drift,&cxs,4400u,0u,0u)==0u);
 cxs.mode=RU_TOUCH_MODE_PRESS;
 assert(ru_touch_motion(&drift,&cxs,4500u,1u,91u)==91u);
 cxs.mode=RU_TOUCH_MODE_VELOCITY;
 assert(ru_touch_profile_valid(&cxs));
 assert(ru_touch_motion(&drift,&cxs,4700u,1u,91u)==0u);

 struct rupture_fx_chain chain;ru_chain_clear(&chain);
 ru_chain_insert(&chain,0u,RU_FX_DISTORTION);
 uint16_t drive=chain.parameter[0][0];
 assert(ru_touch_ensure_reverb(&chain,30u));
 assert(chain.type[0]==RU_FX_DISTORTION&&chain.parameter[0][0]==drive);
 assert(chain.type[1]==RU_FX_REVERB&&chain.enabled==3u);
 assert(chain.parameter[1][3]==ru_touch_percent_q15(30u));
 assert(ru_touch_ensure_reverb(&chain,45u));
 assert(chain.type[1]==RU_FX_REVERB&&chain.parameter[1][3]==ru_touch_percent_q15(45u));
 struct ru_touch_profile rv={RU_TOUCH_REVERB,RU_TOUCH_MODE_PRESS,2u,45u,90u,1u};
 assert(ru_touch_level_percent(&rv,0u)==45u);
 assert(ru_touch_level_percent(&rv,127u)==90u);
 assert(ru_touch_level_percent(&rv,64u)>45u);
 c=base;c.fx_chain=chain;
 ru_touch_apply_profile(&c,&rv,0u);
 assert(c.fx_chain.parameter[1][3]==ru_touch_percent_q15(45u));
 ru_touch_apply_profile(&c,&rv,127u);
 assert(c.fx_chain.parameter[1][3]==ru_touch_percent_q15(90u));
 assert(chain.parameter[1][3]==ru_touch_percent_q15(45u)); /* saved baseline untouched */
 for(uint8_t i=0;i<RU_FX_SLOTS;i++)ru_chain_insert(&chain,i,RU_FX_MOSFET);
 struct rupture_fx_chain full=chain;
 assert(!ru_touch_ensure_reverb(&chain,65u));
 assert(!memcmp(&chain,&full,sizeof(chain))); /* full chain: never replace FX */
 assert(!ru_touch_profile_valid(&(struct ru_touch_profile){RU_TOUCH_REVERB,0u,1u,70u,30u,1u}));
 puts("PASS V2.0.58 separate CXS/GROWL profiles, smoothed drift, 0..100 range, safe reverb auto-slot + full guard");
 return 0;
}
