/* Global pressure target+feel remains independent of presets and projects.
 * Uses the real NOR snapshot/journal implementation with simulated flash. */
#define main earlier_global_settings_tests
#include "test_v127_global_settings.c"
#undef main
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_key_touch.h"
int main(void){
 earlier_global_settings_tests();
 uint8_t p=255u,rate=255u,cxs=255u,growl=255u;
 assert(etch_store_touch_load(&io,&p)==ETCH_STORE_OK&&p==8u);
 assert(etch_store_touch_save(&io,ru_touch_config(RU_TOUCH_GROWL,3u))==ETCH_STORE_OK);
 assert(etch_store_touch_load(&io,&p)==ETCH_STORE_OK&&p==ru_touch_config(RU_TOUCH_GROWL,3u));
 assert(etch_store_usb_rate_save(&io,1u)==ETCH_STORE_OK);
 assert(etch_store_touch_load(&io,&p)==ETCH_STORE_OK&&p==ru_touch_config(RU_TOUCH_GROWL,3u));
 assert(etch_store_preset_assign_pair_save(&io,42u,93u)==ETCH_STORE_OK);
 assert(etch_store_touch_load(&io,&p)==ETCH_STORE_OK&&p==ru_touch_config(RU_TOUCH_GROWL,3u));
 assert(etch_store_usb_rate_load(&io,&rate)==ETCH_STORE_OK&&rate==1u);
 save_project(6);check_project(6);
 assert(etch_store_touch_load(&io,&p)==ETCH_STORE_OK&&p==ru_touch_config(RU_TOUCH_GROWL,3u));
 assert(etch_store_preset_assign_pair_load(&io,&cxs,&growl)==ETCH_STORE_OK&&cxs==42u&&growl==93u);
 fail_after=0;writes=0;
 assert(etch_store_touch_save(&io,ru_touch_config(RU_TOUCH_FILTER,0u))==ETCH_STORE_IO);
 fail_after=-1;
 assert(etch_store_touch_load(&io,&p)==ETCH_STORE_OK&&p==ru_touch_config(RU_TOUCH_GROWL,3u));
 assert(etch_store_touch_save(&io,8u)==ETCH_STORE_OK);
 assert(etch_store_touch_load(&io,&p)==ETCH_STORE_OK&&p==8u);
 assert(etch_store_touch_save(&io,32u)==ETCH_STORE_IO);
 assert(etch_store_touch_save(&io,ru_touch_config(RU_TOUCH_REVERB,2u))==ETCH_STORE_OK);
 for(uint32_t i=0;i<40u;i++)volume((uint8_t)(120u+(i&1u)));
 assert(etch_store_touch_load(&io,&p)==ETCH_STORE_OK&&p==ru_touch_config(RU_TOUCH_REVERB,2u));
 assert(etch_store_usb_rate_load(&io,&rate)==ETCH_STORE_OK&&rate==1u);
 assert(etch_store_preset_assign_pair_load(&io,&cxs,&growl)==ETCH_STORE_OK&&cxs==42u&&growl==93u);
 puts("PASS V2.0.58 pressure target/sensitivity persistence and torn-write recovery");
 return 0;
}
