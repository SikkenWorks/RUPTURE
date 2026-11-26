/* Simulated physical flash: two independent CRC-checked global records,
 * backward-compatible upgrade, torn writes, project rewrites/compaction. */
#define main earlier_global_settings_tests
#include "test_v127_global_settings.c"
#undef main
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_key_touch.h"
int main(void){
 earlier_global_settings_tests();
 assert(etch_store_touch_save(&io,ru_touch_config(RU_TOUCH_REVERB,2u))==ETCH_STORE_OK);
 struct ru_touch_profile c={RU_TOUCH_REVERB,RU_TOUCH_MODE_PRESS,1u,23u,81u,1u};
 struct ru_touch_profile g={RU_TOUCH_GROWL,RU_TOUCH_MODE_RANDOM,3u,4u,62u,3u};
 struct ru_touch_profile read={0};
 assert(etch_store_touch_profile_load(&io,0u,&read)==ETCH_STORE_OK);
 assert(read.target==RU_TOUCH_REVERB&&read.mode==RU_TOUCH_MODE_PRESS);
 assert(read.min_pct==0u&&read.max_pct==100u); /* inherited V247 */
 assert(etch_store_touch_profile_save(&io,0u,&c)==ETCH_STORE_OK);
 assert(etch_store_touch_profile_save(&io,1u,&g)==ETCH_STORE_OK);
 assert(etch_store_touch_profile_load(&io,0u,&read)==ETCH_STORE_OK&&ru_touch_profile_equal(&read,&c));
 assert(etch_store_touch_profile_load(&io,1u,&read)==ETCH_STORE_OK&&ru_touch_profile_equal(&read,&g));
 assert(etch_store_touch_profile_save(&io,0u,&(struct ru_touch_profile){7u,1u,0u,90u,20u,1u})==ETCH_STORE_IO);
 assert(etch_store_touch_profile_save(&io,2u,&g)==ETCH_STORE_IO);
 assert(etch_store_usb_rate_save(&io,0u)==ETCH_STORE_OK);
 assert(etch_store_preset_assign_pair_save(&io,7u,85u)==ETCH_STORE_OK);
 save_project(3);check_project(3);
 assert(etch_store_touch_profile_load(&io,0u,&read)==ETCH_STORE_OK&&ru_touch_profile_equal(&read,&c));
 assert(etch_store_touch_profile_load(&io,1u,&read)==ETCH_STORE_OK&&ru_touch_profile_equal(&read,&g));
 fail_after=0;writes=0;
 struct ru_touch_profile changed=g;changed.min_pct=31u;
 assert(etch_store_touch_profile_save(&io,1u,&changed)==ETCH_STORE_IO);
 fail_after=-1;
 assert(etch_store_touch_profile_load(&io,1u,&read)==ETCH_STORE_OK&&ru_touch_profile_equal(&read,&g));
 for(uint32_t i=0;i<44u;i++)volume((uint8_t)(114u+(i&1u)));
 assert(etch_store_touch_profile_load(&io,0u,&read)==ETCH_STORE_OK&&ru_touch_profile_equal(&read,&c));
 assert(etch_store_touch_profile_load(&io,1u,&read)==ETCH_STORE_OK&&ru_touch_profile_equal(&read,&g));
 puts("PASS V2.0.58 two CRC profiles survive torn NOR writes, compaction, presets, USB rate and project switch");
 return 0;
}
