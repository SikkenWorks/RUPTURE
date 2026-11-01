/* Regression for RUPTURE V1.43 against the actual native OTA C source.
 * Existing M-UPGRADE SysEx simulator serves real .fwsc, and mock NOR is
 * seeded with a structurally recognized loader left by an older install.
 */
#include <stdint.h>
static void ota_progress_update(uint32_t done,uint32_t total) {(void)done;(void)total;}
#define main v143_unused_legacy_main
#include "../vendor/etch-v1.88-source/tests/test_native_ota.c"
#undef main

static int read_package(const char *path){
 FILE *f=fopen(path,"rb");if(!f)return 0;
 uint8_t *raw=malloc(0x200000u);
 size_t n=fread(raw,1,0x200000u,f);fclose(f);
 if(n<0x4400u){free(raw);return 0;}
 logical=malloc(n);
 for(size_t i=0;i<20;i++)memcpy(logical+i*47u,raw+i*48u,47u);
 memcpy(logical+940u,raw+960u,n-960u);
 logical_len=n-20u;free(raw);return 1;
}
int main(int argc,char **argv){
 if(argc!=2||!read_package(argv[1])){fputs("Pass a V1.42 .fwsc fixture\n",stderr);return 2;}
 int bad=0,rc;
 reset_dev();rc=ota_session();
 bad+=check("blank stage: valid app-only update commits",rc==0&&committed&&!bad_erase);
 reset_dev();
 /* Complete trusted loader + harmless trailing padding: a different, still
  * bounded LOADER.BIN footprint not matching RUPTURE's pinned hash policy.
  * The inner valid loader is never executed, only replaced. */
 memcpy(nor+OTA_AREA+32u,etch_trusted_app_loader,ETCH_OTA_BODY_LEN);
 memset(nor+OTA_AREA+32u+ETCH_OTA_BODY_LEN,0xff,32u);
 ota_stage_head(ETCH_OTA_BODY_LEN+32u);
 bad+=check("recognizes bounded residual loader",etch_ota_available(ota_dec));
 rc=ota_session();
 bad+=check("reclaims recognized stage, commits valid destination",rc==0&&committed&&!bad_erase);
 bad+=check("staged executable is pinned trusted app-only loader",
             !memcmp(nor+OTA_AREA+32u,etch_trusted_app_loader,ETCH_OTA_BODY_LEN));
 reset_dev();nor[OTA_AREA+1000u]=0x81;rc=ota_session();
 bad+=check("unknown nonblank staging blocks without erasing",rc==-15&&!committed&&nor[OTA_AREA+1000u]==0x81);
 reset_dev();
 memcpy(nor+OTA_AREA+32u,etch_trusted_app_loader,ETCH_OTA_BODY_LEN);
 ota_stage_head(ETCH_OTA_BODY_LEN+32u);
 nor[OTA_AREA+32u+8u]^=1u;rc=ota_session();
 bad+=check("invalid loader inner header blocks without erase",rc==-15&&!committed);
 reset_dev();
 memcpy(nor+OTA_AREA+32u,etch_trusted_app_loader,ETCH_OTA_BODY_LEN);
 ota_stage_head(ETCH_OTA_BODY_LEN+32u);
 nor[OTA_AREA+0u]^=1u;rc=ota_session();
 bad+=check("invalid JLFS CRC blocks without erase",rc==-15&&!committed);
 /* Test stock head check itself: deliberately different installed image
  * must fail, without touching any staging sector. Whole-stock session
  * replay requires exact official V15 file, not bundled in our project. */
 reset_dev();
 uint8_t host_head[512],board_head[512];
 bad+=check("identical boothead accepted",
             etch_stock_boot_head_matches(0x400u,host_head,board_head));
 nor[0x120u]^=0x01u;
 bad+=check("different boothead rejected",
             !etch_stock_boot_head_matches(0x400u,host_head,board_head));
 bad+=check("boot guard did not erase flash",!bad_erase&&area_erased());
 free(logical);printf("RUPTURE V1.43 native OTA: %s\n",bad?"FAIL":"PASS");return !!bad;
}
