/* Device-global 44.1/48 kHz preference must survive project saves,
 * assignment changes, journal compaction and a torn NOR write. */
#define main earlier_global_settings_tests
#include "test_v127_global_settings.c"
#undef main
int main(void){
 earlier_global_settings_tests();
 uint8_t rate=255u,cxs=255u,growl=255u;
 assert(etch_store_usb_rate_load(&io,&rate)==ETCH_STORE_OK && rate==0u);
 assert(etch_store_usb_rate_save(&io,1u)==ETCH_STORE_OK);
 assert(etch_store_usb_rate_load(&io,&rate)==ETCH_STORE_OK && rate==1u);
 assert(etch_store_preset_assign_pair_save(&io,35u,91u)==ETCH_STORE_OK);
 assert(etch_store_preset_assign_pair_load(&io,&cxs,&growl)==ETCH_STORE_OK);
 assert(cxs==35u&&growl==91u);
 assert(etch_store_usb_rate_load(&io,&rate)==ETCH_STORE_OK && rate==1u);
 save_project(5);check_project(5);
 volume(123u);
 assert(etch_store_usb_rate_load(&io,&rate)==ETCH_STORE_OK && rate==1u);
 /* Legacy 8-byte record's CRC catches corruption; no partial rate update. */
 fail_after=0;writes=0;
 assert(etch_store_usb_rate_save(&io,0u)==ETCH_STORE_IO);
 fail_after=-1;
 assert(etch_store_usb_rate_load(&io,&rate)==ETCH_STORE_OK && rate==1u);
 assert(etch_store_usb_rate_save(&io,0u)==ETCH_STORE_OK);
 assert(etch_store_usb_rate_load(&io,&rate)==ETCH_STORE_OK && rate==0u);
 assert(etch_store_usb_rate_save(&io,2u)==ETCH_STORE_IO);
 assert(etch_store_preset_assign_pair_load(&io,&cxs,&growl)==ETCH_STORE_OK);
 assert(cxs==35u&&growl==91u);
 puts("PASS V2.0.31 USB preference 44.1/48 stored; presets and projects intact");
 return 0;
}
