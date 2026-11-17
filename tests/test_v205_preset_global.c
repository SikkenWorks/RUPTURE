/* Reuse the full established fake-NOR project+settings storage model. */
#define main test_v127_existing_global_journal
#include "test_v127_global_settings.c"
#undef main
int main(void){
 test_v127_existing_global_journal();
 uint8_t assign=255u,vol=255u,autosave=255u;
 assert(etch_store_preset_assign_load(&io,&assign)==ETCH_STORE_OK);
 assert(assign==1u); /* legacy device default: BPM */
 assert(etch_store_preset_assign_save(&io,36u)==ETCH_STORE_OK);
 int probe=etch_store_preset_assign_load(&io,&assign);
 if(probe!=ETCH_STORE_OK||assign!=36u)fprintf(stderr,"mapping probe rc=%d got=%u expected=36\n",probe,assign);
 assert(probe==ETCH_STORE_OK&&assign==36u);
 save_project(4);check_project(4); /* device mapping independent of slots */
 assert(etch_store_preset_assign_load(&io,&assign)==ETCH_STORE_OK&&assign==36u);
 assert(etch_store_global_settings_load(&io,&vol,&autosave)==ETCH_STORE_OK);
 assert(vol==199u&&autosave==1u);
 fail_after=0;writes=0;
 assert(etch_store_preset_assign_save(&io,39u)==ETCH_STORE_IO);
 fail_after=-1;
 assert(etch_store_preset_assign_load(&io,&assign)==ETCH_STORE_OK&&assign==36u);
 /* Repeated changes force A/B journal compaction; settings and all project
  * payloads remain readable and the final global selection remains intact. */
 for(uint8_t n=0u;n<42u;n++){
  uint8_t new_choice=(uint8_t)(20u+(n%40u));
  assert(etch_store_preset_assign_save(&io,new_choice)==ETCH_STORE_OK);
  assert(etch_store_preset_assign_load(&io,&assign)==ETCH_STORE_OK&&assign==new_choice);
 }
 for(uint8_t i=0u;i<5u;i++)check_project(i);
 assert(etch_store_global_settings_load(&io,&vol,&autosave)==ETCH_STORE_OK);
 assert(vol==199u&&autosave==1u);
 volume(145u); /* Compaction of regular settings also keeps mapping. */
 assert(etch_store_preset_assign_load(&io,&assign)==ETCH_STORE_OK&&assign==21u);
 assert(etch_store_preset_assign_save(&io,96u)==ETCH_STORE_IO);
 puts("PASS V2.0.5: PRESETS mapping globally stored, project-independent, CRC/torn safe, both A/B compaction directions");
 return 0;
}
