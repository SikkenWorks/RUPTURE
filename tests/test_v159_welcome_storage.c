/* Exercise existing 1 MiB flash emulator without changing its original tests. */
#define main prior_storage_main
#include "test_v127_global_settings.c"
#undef main
int main(void){
 assert(prior_storage_main()==0);
 uint8_t volume=0,autosave=0,saver=0,seconds=0,boot=0,crt=0,done=3;
 assert(etch_store_user_settings_welcome_load(&io,&volume,&autosave,&saver,&seconds,&boot,&crt,&done)==ETCH_STORE_OK);
 assert(done==0u);
 assert(etch_store_user_settings_welcome_save(&io,volume,autosave,saver,seconds,boot,1u,1u)==ETCH_STORE_OK);
 assert(etch_store_user_settings_welcome_load(&io,&volume,&autosave,&saver,&seconds,&boot,&crt,&done)==ETCH_STORE_OK);
 assert(done==1u&&crt==1u);
 /* Old public APIs must preserve the completed wizard marker. */
 assert(etch_store_user_settings_crt_save(&io,155u,autosave,saver,seconds,boot,0u)==ETCH_STORE_OK);
 assert(etch_store_global_volume_save(&io,122u)==ETCH_STORE_OK);
 assert(etch_store_user_settings_welcome_load(&io,&volume,&autosave,&saver,&seconds,&boot,&crt,&done)==ETCH_STORE_OK);
 assert(done==1u&&crt==0u&&volume==122u);
 save_project(4);
 assert(etch_store_user_settings_welcome_load(&io,&volume,&autosave,&saver,&seconds,&boot,&crt,&done)==ETCH_STORE_OK);
 assert(done==1u&&crt==0u&&volume==122u);
 /* Journal compaction, bank swap, torn write. */
 for(int i=0;i<38;i++)assert(etch_store_global_volume_save(&io,(uint8_t)(60+i))==ETCH_STORE_OK);
 assert(etch_store_user_settings_welcome_load(&io,&volume,&autosave,&saver,&seconds,&boot,&crt,&done)==ETCH_STORE_OK);
 assert(done==1u&&volume==97u);
 fail_after=0;writes=0;
 assert(etch_store_user_settings_welcome_save(&io,133u,autosave,saver,seconds,boot,0u,1u)==ETCH_STORE_IO);
 fail_after=-1;
 assert(etch_store_user_settings_welcome_load(&io,&volume,&autosave,&saver,&seconds,&boot,&crt,&done)==ETCH_STORE_OK);
 assert(done==1u&&volume==97u);
 /* Optional first-run format must only initialize empty owned RUPTURE banks,
  * leaving the application, boot, OTA and unrelated FM1 areas untouched. */
 memset(flashbuf,255,sizeof flashbuf);
 make_project(0);
 etch_store_project_select(0u);
 assert(etch_store_initialize_project(&io,&l,&p,&cfg,ETCH_STORE_ERASE_CONFIRM)==ETCH_STORE_OK);
 for(uint32_t at=0;at<ETCH_STORE_OWNED_BASE;at++)assert(flashbuf[at]==255u);
 for(uint32_t at=ETCH_STORE_OWNED_END;at<N;at++)assert(flashbuf[at]==255u);
 assert(etch_store_load_project(&io,&out,&op,&ocfg)==ETCH_STORE_OK);
 assert(etch_store_initialize_project(&io,&l,&p,&cfg,ETCH_STORE_ERASE_CONFIRM)==ETCH_STORE_INCOMPATIBLE);
 for(uint32_t at=0;at<ETCH_STORE_OWNED_BASE;at++)assert(flashbuf[at]==255u);
 for(uint32_t at=ETCH_STORE_OWNED_END;at<N;at++)assert(flashbuf[at]==255u);
 puts("PASS V1.59 confirmed empty-storage initialization limited to owned banks, no overwrite of valid storage");
 puts("PASS V1.59 completed wizard marker survives project writes, old settings calls, A/B compaction and torn writes");
 return 0;
}
