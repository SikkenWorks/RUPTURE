/* Regression: legacy completion is not mistaken for the V1.82 setup completion.
 * The CRC-protected project catalog and all other settings survive migration. */
#define main old_test_main
#include "test_v127_global_settings.c"
#undef main
static void read_w(uint8_t *done,uint8_t *current,uint8_t *vol,uint8_t *crt){
 uint8_t a=0,s=0,t=0,b=0;
 assert(etch_store_user_settings_welcome_version_load(&io,vol,&a,&s,&t,&b,crt,done,current)==ETCH_STORE_OK);
}
int main(void){
 memset(flashbuf,255,sizeof flashbuf);save_project(0);save_project(1);
 assert(etch_store_user_settings_welcome_save(&io,123,1,1,2,1,1,1)==ETCH_STORE_OK);
 uint8_t done=0,current=0,vol=0,crt=0;read_w(&done,&current,&vol,&crt);
 assert(done==1&&current==1&&vol==123&&crt==1);
 /* Simulate the exact legacy V1.81 record on NOR by changing only the
  * record's magic and recalculating the 4-byte CRC, not touching projects. */
 int found=0;
 for(uint32_t i=0;i<N-8;i++){
  uint16_t m=(uint16_t)(flashbuf[i]|((uint16_t)flashbuf[i+1]<<8));
  if(m>=0x6000u&&m<0x6000u+13u*32u && flashbuf[i+2]==123u&&flashbuf[i+3]==(uint8_t)(123u^255u)){
   uint16_t old=(uint16_t)(m-0x6000u+0x5c00u);
   flashbuf[i]=(uint8_t)old;flashbuf[i+1]=(uint8_t)(old>>8);
   uint32_t crc=etch_store_crc(&flashbuf[i],4);
   for(unsigned k=0;k<4;k++)flashbuf[i+4+k]=(uint8_t)(crc>>(k*8));
   found++;
  }
 }
 assert(found==1);
 read_w(&done,&current,&vol,&crt);
 assert(done==1&&current==0&&vol==123&&crt==1);
 /* Completing the setup writes a NEW record even if the user changed no
  * preferences. Next cold boot sees current=1 and must skip wizard. */
 assert(etch_store_user_settings_welcome_save(&io,123,1,1,2,1,1,1)==ETCH_STORE_OK);
 read_w(&done,&current,&vol,&crt);
 assert(done==1&&current==1&&vol==123&&crt==1);
 check_project(0);check_project(1);
 /* SAVE interruption cannot fake a successful completed setup. */
 assert(etch_store_user_settings_welcome_save(&io,123,1,1,2,1,1,0)==ETCH_STORE_OK);
 fail_after=0;writes=0;
 assert(etch_store_user_settings_welcome_save(&io,123,1,1,2,1,1,1)==ETCH_STORE_IO);
 fail_after=-1;
 read_w(&done,&current,&vol,&crt);assert(done==0&&current==0);
 assert(etch_store_user_settings_welcome_save(&io,123,1,1,2,1,1,1)==ETCH_STORE_OK);
 read_w(&done,&current,&vol,&crt);assert(done==1&&current==1);
 puts("PASS V1.82 legacy wizard migration, next boot, CRC, projects, torn write");return 0;
}
