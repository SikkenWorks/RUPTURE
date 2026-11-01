/* Native stock V15 OTA regression: real FM-1_015 vendor file and a RUPTURE
 * installed boot area are supplied at test time; no proprietary stock bytes
 * are distributed in the RUPTURE release ZIP. */
#include <stdint.h>
static void ota_progress_update(uint32_t done,uint32_t total){(void)done;(void)total;}
#define main v144_legacy_test_main
#include "../vendor/etch-v1.88-source/tests/test_native_ota.c"
#undef main

static int package_read(const char *path){
 FILE *f=fopen(path,"rb");if(!f)return 0;
 uint8_t *raw=malloc(0x200000u);
 size_t n=fread(raw,1,0x200000u,f);fclose(f);
 if(n<0x4400u){free(raw);return 0;}
 free(logical);logical=malloc(n);
 for(size_t i=0;i<20u;i++)memcpy(logical+i*47u,raw+i*48u,47u);
 memcpy(logical+940u,raw+960u,n-960u);
 logical_len=n-20u;free(raw);return 1;
}
static int load_boot_head(const char *path){
 FILE *f=fopen(path,"rb");if(!f)return 0;
 uint8_t raw[0x400u+20u+0x4000u];size_t n=fread(raw,1,sizeof raw,f);fclose(f);
 if(n!=sizeof raw)return 0;
 /* First 960 bytes of raw package contain 20 embedded identity markers.
  * Firmware boot region starts at logical package offset 0x400. */
 memcpy(installed_head,raw+960u+(0x400u-940u),0x4000u); /* boot area entirely past first 940 bytes */
 return 1;
}
int main(int argc,char **argv){
 if(argc!=3 || !load_boot_head(argv[2]) || !package_read(argv[1])){
  fprintf(stderr,"Usage: test_v144_stock_v15 STOCK-V15.fwsc RUPTURE.fwsc\n");return 2;
 }
 int bad=0,rc;
 test_cross_install=1;
 reset_dev();rc=ota_session();
 printf("V15 normal V1.44 native session rc=%d, reads=%u, last_code=%d\n",rc,requests,last_code);
 bad+=check("original stock V15: full 8-entry container and approved boot accepted",rc==0&&committed&&!bad_erase);
 bad+=check("official loader was staged, not a foreign executable",!memcmp(nor+OTA_AREA+32u,logical+0xa5f00u+32u,ETCH_STOCK_OTA_BODY_LEN));
 bad+=check("stage header programmed",ota_rd32(nor+OTA_AREA+4u)==OTA_AREA);
 bad+=check("target FM1 boot unchanged before authorized commit",!memcmp(nor,installed_head,0x4000u));
 reset_dev();nor[0x1000u]^=0x01u;rc=ota_session();
 printf("V15 mismatched uboot rc=%d\n",rc);
 bad+=check("unknown boot-content mismatch: fail closed",rc==-5&&!committed&&!bad_erase&&area_erased());
 reset_dev();nor[0x20u]^=0x01u;rc=ota_session();
 bad+=check("unknown boot directory signature: fail closed",rc==-5&&!committed&&!bad_erase);
 reset_dev();logical[0x404u]^=0x01u;rc=ota_session();
 bad+=check("damaged stock flash: fail before erase",rc==-22&&!committed&&!bad_erase);
 logical[0x404u]^=0x01u;
 reset_dev();logical[0x42u]^=0x01u;rc=ota_session();
 bad+=check("damaged stock metadata: header CRC refuses package",rc==-2&&!committed&&!bad_erase);
 logical[0x42u]^=0x01u;
 reset_dev();nor[OTA_AREA+1000u]=0x81u;rc=ota_session();
 bad+=check("unknown staging still protected",rc==-15&&!committed&&!bad_erase);
 free(logical);
 printf("V1.44 native real V15 replay: %s\n",bad?"FAIL":"PASS");return !!bad;
}
