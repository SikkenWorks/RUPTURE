/* Verify CXS/GROWL mappings survive interleaved A/B project/settings writes. */
#define main earlier_global_settings_tests
#include "test_v127_global_settings.c"
#undef main
int main(void){
 earlier_global_settings_tests();
 uint8_t cxs=255u,growl=255u;
 assert(etch_store_preset_assign_pair_load(&io,&cxs,&growl)==ETCH_STORE_OK);
 assert(cxs==1u&&growl==94u);
 assert(etch_store_preset_assign_pair_save(&io,21u,6u)==ETCH_STORE_OK);
 assert(etch_store_preset_assign_pair_load(&io,&cxs,&growl)==ETCH_STORE_OK);
 assert(cxs==21u&&growl==6u);
 /* Existing single-CXS API is still valid and does not change GROWL. */
 assert(etch_store_preset_assign_save(&io,15u)==ETCH_STORE_OK);
 assert(etch_store_preset_assign_pair_load(&io,&cxs,&growl)==ETCH_STORE_OK);
 assert(cxs==15u&&growl==6u);
 save_project(4);check_project(4);
 assert(etch_store_preset_assign_pair_load(&io,&cxs,&growl)==ETCH_STORE_OK);
 assert(cxs==15u&&growl==6u);
 /* Failure must leave both previous journal values unchanged. */
 fail_after=0;writes=0;
 assert(etch_store_preset_assign_pair_save(&io,19u,94u)==ETCH_STORE_IO);
 fail_after=-1;
 assert(etch_store_preset_assign_pair_load(&io,&cxs,&growl)==ETCH_STORE_OK);
 assert(cxs==15u&&growl==6u);
 for(uint8_t i=0u;i<44u;i++){
  uint8_t new_cxs=(uint8_t)(6u+(i%20u));
  uint8_t new_growl=(uint8_t)(94u-(i%10u));
  assert(etch_store_preset_assign_pair_save(&io,new_cxs,new_growl)==ETCH_STORE_OK);
  assert(etch_store_preset_assign_pair_load(&io,&cxs,&growl)==ETCH_STORE_OK);
  assert(cxs==new_cxs&&growl==new_growl);
 }
 volume(137u); /* Settings journal compaction must keep both assignments. */
 assert(etch_store_preset_assign_pair_load(&io,&cxs,&growl)==ETCH_STORE_OK);
 assert(cxs==9u&&growl==91u);
 assert(etch_store_preset_assign_pair_save(&io,94u,96u)==ETCH_STORE_IO);
 assert(etch_store_preset_assign_pair_save(&io,15u,95u)==ETCH_STORE_OK);
 assert(etch_store_preset_assign_pair_load(&io,&cxs,&growl)==ETCH_STORE_OK);
 assert(cxs==15u&&growl==95u);
 puts("PASS V2.0.13: CXS+GROWL independent global knobs, A/B compaction, torn writes");
 return 0;
}
