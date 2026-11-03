/* SPDX-License-Identifier: GPL-3.0-only
 * Two snapshot banks. Payload first, verified commit header last.
 */
#include "wave_store.h"
#include "native_project_codec.h"
static void (*ru_store_progress_fn)(uint8_t,uint32_t,uint32_t);
void etch_store_set_progress(void (*fn)(uint8_t,uint32_t,uint32_t)){
 ru_store_progress_fn=fn;
}
static void ru_store_progress(uint8_t phase,uint32_t done,uint32_t total){
 if(ru_store_progress_fn && total)ru_store_progress_fn(phase,done,total);
}
/* Stages: 1=erase, 2=program, 3=read-back verify, 4=project load. */
#define MAGIC 0x38575745u /* EWW8: separate from every Felucca object */
#define PAYLOAD_OFF 256u
#ifndef ETCH_P05_MULTI
#define ETCH_P05_MULTI 0
#endif
#define ETCH_P05_SLOTS 12u
#define ETCH_P05_LEGACY_SLOTS 4u
#define ETCH_P05_V13_MAGIC 0x32354A50u /* PJ52 */
#define ETCH_P05_MAGIC 0x31354A50u /* PJ51 */
struct p05_item {uint32_t offset,bytes,crc;char name[12];};
struct p05_manifest {uint32_t magic,slots;struct p05_item item[ETCH_P05_SLOTS];};
struct p05_legacy_manifest {uint32_t magic,slots;struct p05_item item[ETCH_P05_LEGACY_SLOTS];};
_Static_assert(sizeof(struct p05_legacy_manifest)==104u,"legacy catalog ABI");
static uint8_t project_slot=0,requested_slot=255,global_wave_pending=0;
#define GLOBAL_OWNER_MAGIC 0x474C0000u
static uint8_t project_current_slot(uint32_t x){return (uint8_t)(x&255u);}
static uint8_t project_global_slot(uint32_t x){return (x&0xffff0000u)==GLOBAL_OWNER_MAGIC?(uint8_t)((x>>8)&255u):255u;}
void etch_store_global_wave_save(void){global_wave_pending=1u;}
void etch_store_global_wave_cancel(void){global_wave_pending=0u;}
static char project_name[12]="PROJECT 1";
void etch_store_project_select(uint8_t index){if(index<ETCH_P05_SLOTS)project_slot=index;}
uint8_t etch_store_project_selected(void){return project_slot;}
/* Validate each catalog version using its actual header size, not a guessed
 * cast to the new twelve-entry structure. Strict adjacency prevents aliasing
 * another record or reading beyond the CRC-verified payload. */
static int p05_valid_range(const struct p05_manifest *m,uint32_t length,uint32_t header){
 if(m->slots!=ETCH_P05_SLOTS)return 0;
 uint32_t end=header;
 for(uint32_t i=0;i<ETCH_P05_SLOTS;i++){
  const struct p05_item *it=&m->item[i];
  if(!it->bytes){if(it->offset||it->crc)return 0;continue;}
  if(it->bytes<64u||it->offset!=end||it->bytes>length-end)return 0;
  end+=it->bytes;
 }return end==length;
}
static int p05_manifest_valid(const struct p05_manifest *m,uint32_t length){
 return m->magic==ETCH_P05_V13_MAGIC&&p05_valid_range(m,length,sizeof(*m));
}
static int p05_old_manifest_valid(const struct p05_legacy_manifest *m,uint32_t length){
 if(m->magic!=ETCH_P05_MAGIC||m->slots!=ETCH_P05_LEGACY_SLOTS)return 0;
 uint32_t end=sizeof(*m);
 for(uint32_t i=0;i<ETCH_P05_LEGACY_SLOTS;i++){
  const struct p05_item *it=&m->item[i];
  if(!it->bytes){if(it->offset||it->crc)return 0;continue;}
  if(it->bytes<64u||it->offset!=end||it->bytes>length-end)return 0;
  end+=it->bytes;
 }return end==length;
}

struct header {uint32_t magic,version,generation,bytes,crc,bank,reserved,hcrc;};
_Static_assert(sizeof(struct header)==32,"commit header size");
_Static_assert(sizeof(struct etch_wave_library)<=ETCH_STORE_BANK_BYTES-PAYLOAD_OFF,"wave snapshot fits bank");
static struct etch_wave_library scratch __attribute__((section(".pool")));
static uint8_t packed[ETCH_PROJECT_MAX_BYTES] __attribute__((section(".pool")));
static struct etch_native_pattern scratch_pattern __attribute__((section(".pool")));
static struct etch_project_settings scratch_settings;
static struct etch_preset_dsp_bank scratch_dsp;
uint32_t etch_store_crc(const void *src,uint32_t bytes){
 const uint8_t *p=src;uint32_t c=~0u;
 while(bytes--){c^=*p++;for(uint32_t i=0;i<8;i++)c=(c>>1)^((0u-(c&1u))&0xedb88320u);}
 return ~c;
}
/* Region 0: legacy, region 1: P04 expansion, region 2: verified-blank spare. */
static uint8_t store_region;
static uint8_t store_takeover_active;
uint8_t etch_store_active_region(void){return store_region;}
static uint32_t region_base(void){
#if ETCH_STORE_OWNED_ENABLED
 if(store_region==3u)return ETCH_STORE_OWNED_BASE;
#endif
#if ETCH_STORE_SPARE_ENABLED
 if(store_region==2u)return ETCH_STORE_SPARE_BASE;
#endif
#if ETCH_P04_EXPANDED
 return store_region ? ETCH_STORE_EXT_BASE : ETCH_STORE_BASE;
#else
 return ETCH_STORE_BASE;
#endif
}
static uint32_t region_bank_bytes(void){
#if ETCH_STORE_OWNED_ENABLED
 if(store_region==3u)return ETCH_STORE_OWNED_BANK_BYTES;
#endif
#if ETCH_STORE_SPARE_ENABLED
 if(store_region==2u)return ETCH_STORE_SPARE_BANK_BYTES;
#endif
#if ETCH_P04_EXPANDED
 return store_region ? ETCH_STORE_EXT_BANK_BYTES : ETCH_STORE_BANK_BYTES;
#else
 return ETCH_STORE_BANK_BYTES;
#endif
}
static uint32_t base(uint32_t bank){return region_base()+bank*region_bank_bytes();}
static int head(const struct etch_store_io *io,uint32_t bank,struct header *h){
 if(io->read(base(bank),h,sizeof(*h)))return ETCH_STORE_IO;
 return h->magic==MAGIC&&(h->version==1||h->version==2||h->version==3||h->version==4||h->version==5||h->version==6||h->version==7||h->version==8||h->version==9||h->version==10||h->version==11||h->version==12||h->version==13)&&h->bank==bank&&((h->version==3||h->version==4||h->version==5||h->version==6||h->version==7||h->version==8||h->version==9||h->version==10||h->version==11||h->version==12||h->version==13)?(h->bytes>=64&&h->bytes<=region_bank_bytes()-PAYLOAD_OFF):(h->bytes==(h->version==1?ETCH_STORE_V1_BYTES:sizeof(scratch))))&&h->hcrc==etch_store_crc(h,28)?0:1;
}
static int p05_read_catalog(const struct etch_store_io *io,uint32_t bank,
 const struct header *h,struct p05_manifest *dest){
 if(h->version==13u){
  if(h->bytes<sizeof(*dest)||io->read(base(bank)+PAYLOAD_OFF,dest,sizeof(*dest)))return ETCH_STORE_VERIFY;
  return p05_manifest_valid(dest,h->bytes)?ETCH_STORE_OK:ETCH_STORE_VERIFY;
 }
 if(h->version==11u||h->version==12u){
  struct p05_legacy_manifest old;
  if(h->bytes<sizeof(old)||io->read(base(bank)+PAYLOAD_OFF,&old,sizeof(old)))return ETCH_STORE_VERIFY;
  if(!p05_old_manifest_valid(&old,h->bytes))return ETCH_STORE_VERIFY;
  *dest=(struct p05_manifest){0};dest->magic=ETCH_P05_V13_MAGIC;dest->slots=ETCH_P05_SLOTS;
  for(uint32_t i=0;i<ETCH_P05_LEGACY_SLOTS;i++)dest->item[i]=old.item[i];
  return ETCH_STORE_OK;
 }
 return ETCH_STORE_INCOMPATIBLE;
}
static int body(const struct etch_store_io *io,uint32_t bank,const struct header *h,
 struct etch_wave_library *out,struct etch_native_pattern *pattern,struct etch_project_settings *settings){
 if(!settings || settings==&scratch_settings)etch_project_bind_preset_dsp(&scratch_settings,&scratch_dsp);
 if(h->version==3||h->version==4||h->version==5||h->version==6||h->version==7||h->version==8||h->version==9||h->version==10||h->version==11||h->version==12||h->version==13){
  if(h->version==11||h->version==12||h->version==13){
   /* CRC the entire multi-project bank in bounded 256-byte reads. Individual
    * project records alone are decoded into the fixed-size RAM buffer. */
   uint8_t page[256];uint32_t crc=~0u;
   for(uint32_t at=0;at<h->bytes;at+=256u){
    uint32_t n=h->bytes-at;if(n>256u)n=256u;
    if(io->read(base(bank)+PAYLOAD_OFF+at,page,n))return ETCH_STORE_IO;
    ru_store_progress(4u,at+n,h->bytes);
    for(uint32_t j=0;j<n;j++){crc^=page[j];for(uint32_t k=0;k<8u;k++)crc=(crc>>1)^((0u-(crc&1u))&0xEDB88320u);}
   }
   if(~crc!=h->crc)return ETCH_STORE_VERIFY;
   struct p05_manifest manifest;
   if(p05_read_catalog(io,bank,h,&manifest)!=ETCH_STORE_OK)return ETCH_STORE_VERIFY;
   uint32_t slot=requested_slot<ETCH_P05_SLOTS?requested_slot:project_current_slot(h->reserved);
   if(slot>=ETCH_P05_SLOTS||!manifest.item[slot].bytes)return ETCH_STORE_VERIFY;
   const struct p05_item *it=&manifest.item[slot];
   if(it->bytes>sizeof(packed))return ETCH_STORE_FULL;
   if(io->read(base(bank)+PAYLOAD_OFF+it->offset,packed,it->bytes))return ETCH_STORE_IO;
   if(etch_store_crc(packed,it->bytes)!=it->crc)return ETCH_STORE_VERIFY;
   if(etch_project_decode(packed,it->bytes,out,pattern?pattern:&scratch_pattern,
        settings?settings:&scratch_settings))return ETCH_STORE_VERIFY;
  }else {
   if(h->bytes>sizeof(packed))return ETCH_STORE_FULL;
   if(io->read(base(bank)+PAYLOAD_OFF,packed,h->bytes))return ETCH_STORE_IO;
   if(etch_store_crc(packed,h->bytes)!=h->crc)return ETCH_STORE_VERIFY;
   if(etch_project_decode(packed,h->bytes,out,pattern?pattern:&scratch_pattern,
        settings?settings:&scratch_settings))return ETCH_STORE_VERIFY;
  }
 }else{
  uint8_t *dst=(uint8_t*)out;for(uint32_t i=0;i<sizeof(*out);i++)dst[i]=0;
  if(io->read(base(bank)+PAYLOAD_OFF,out,h->bytes))return ETCH_STORE_IO;
  if(etch_store_crc(out,h->bytes)!=h->crc)return ETCH_STORE_VERIFY;
  etch_project_legacy(out,pattern,settings);
 }
 for(uint32_t w=0;w<3;w++)if(out->selection[w]>=64)return ETCH_STORE_VERIFY;
 return 0;
}
static int current(const struct etch_store_io *io,struct header *winner,
 struct etch_wave_library *out,struct etch_native_pattern *p,struct etch_project_settings *s){
 struct header h[2];int v[2]={head(io,0,&h[0]),head(io,1,&h[1])};
 if(v[0]==ETCH_STORE_IO||v[1]==ETCH_STORE_IO)return ETCH_STORE_IO;
 uint32_t first=(v[1]==0&&(v[0]!=0||(h[1].generation!=h[0].generation&&h[1].generation-h[0].generation<0x80000000u)))?1:0;
 for(uint32_t n=0;n<2;n++){uint32_t b=first^n;if(!v[b]){int r=body(io,b,&h[b],out,p,s);if(r==ETCH_STORE_IO)return r;if(!r){*winner=h[b];return (int)b;}}}
 return ETCH_STORE_FOREIGN;
}
static int select_project_read_region(const struct etch_store_io *io);
/* RUPTURE V1.13: global-device master volume without another flash sector.
 * Offset 32..255 in each A/B bank was deliberately unused by every
 * PJT32/PJ52 snapshot. Each independently CRC-protected record fits in
 * a single SPI NOR programming page. Torn writes never invalidate the
 * last committed project bank or earlier volume records. */
#define RU_VOL_MAGIC 0x5652u
#define RU_SETTINGS_MAGIC 0x5653u /* legacy AUTO SAVE on; screensaver defaults ON */
#define RU_SAVER_OFF_MAGIC 0x5654u /* autosave OFF, screensaver OFF */
#define RU_BOTH_OFF_MAGIC 0x5655u /* autosave ON, screensaver OFF */
#define RU_SAVER_TIMED_MAGIC 0x5700u
#define RU_BOOT_CRT_OFF_MAGIC 0x5a00u
#define RU_WELCOME_CRT_ON_MAGIC 0x5c00u
#define RU_WELCOME_CRT_OFF_MAGIC 0x5e00u
#define RU_BOOT_MAGIC 0x5800u /* boot project 0=NEW, 1..12 stored project; CRC protected */ /* idx 0..7, low bits ON + AUTO SAVE */
static uint8_t ru_is_timed(uint16_t magic){return magic>=RU_SAVER_TIMED_MAGIC&&magic<RU_SAVER_TIMED_MAGIC+32u;}
static uint8_t ru_is_crt_off_magic(uint16_t m){return (m>=RU_BOOT_CRT_OFF_MAGIC&&m<RU_BOOT_CRT_OFF_MAGIC+13u*32u)||
 (m>=RU_WELCOME_CRT_OFF_MAGIC&&m<RU_WELCOME_CRT_OFF_MAGIC+13u*32u);}
static uint8_t ru_is_welcome_done_magic(uint16_t m){return (m>=RU_WELCOME_CRT_ON_MAGIC&&m<RU_WELCOME_CRT_ON_MAGIC+13u*32u)||
 (m>=RU_WELCOME_CRT_OFF_MAGIC&&m<RU_WELCOME_CRT_OFF_MAGIC+13u*32u);}
static uint8_t ru_is_boot_magic(uint16_t m){return (m>=RU_BOOT_MAGIC&&m<RU_BOOT_MAGIC+13u*32u)||
 (m>=RU_WELCOME_CRT_ON_MAGIC&&m<RU_WELCOME_CRT_ON_MAGIC+13u*32u)||ru_is_crt_off_magic(m);}
static uint16_t ru_boot_payload(uint16_t m){
 return (uint16_t)(m-(ru_is_welcome_done_magic(m)?
 (ru_is_crt_off_magic(m)?RU_WELCOME_CRT_OFF_MAGIC:RU_WELCOME_CRT_ON_MAGIC):
 (ru_is_crt_off_magic(m)?RU_BOOT_CRT_OFF_MAGIC:RU_BOOT_MAGIC)));
}
#define RU_VOL_FIRST 32u
#define RU_VOL_REC_BYTES 8u
#define RU_VOL_SLOTS ((PAYLOAD_OFF-RU_VOL_FIRST)/RU_VOL_REC_BYTES)
struct ru_vol_record {uint16_t magic;uint8_t volume,opposite;uint32_t crc;};
_Static_assert(sizeof(struct ru_vol_record)==8u,"volume record must be exactly 8 bytes");
static struct ru_vol_record ru_vol_make(uint8_t vol,uint8_t autosave,uint8_t screensaver,uint8_t seconds_index,uint8_t boot_project,uint8_t crt,uint8_t welcome_done){
 struct ru_vol_record r={(uint16_t)((welcome_done?(crt?RU_WELCOME_CRT_ON_MAGIC:RU_WELCOME_CRT_OFF_MAGIC):(crt?RU_BOOT_MAGIC:RU_BOOT_CRT_OFF_MAGIC))+((uint16_t)boot_project*32u)+((seconds_index&7u)<<2)+(autosave?2u:0u)+(screensaver?1u:0u)),vol,(uint8_t)(vol^255u),0u};
 r.crc=etch_store_crc(&r,4u);return r;
}
static uint8_t ru_vol_valid(const struct ru_vol_record *r){
 return (r->magic==RU_VOL_MAGIC||r->magic==RU_SETTINGS_MAGIC||r->magic==RU_SAVER_OFF_MAGIC||r->magic==RU_BOTH_OFF_MAGIC||(ru_is_timed(r->magic)||ru_is_boot_magic(r->magic)))&&r->volume<=200u&&
   r->opposite==(uint8_t)(r->volume^255u)&&r->crc==etch_store_crc(r,4u);
}
static int ru_vol_scan(const struct etch_store_io *io,uint32_t bank,
 uint8_t *value,uint8_t *autosave,uint8_t *screensaver,uint8_t *seconds_index,uint8_t *boot_project,uint32_t *free_offset,uint8_t *crt,uint8_t *welcome_done){
 uint32_t last_free=0xffffffffu;*value=100u;if(autosave)*autosave=0u;if(screensaver)*screensaver=1u;if(seconds_index)*seconds_index=1u;if(boot_project)*boot_project=0u;if(crt)*crt=0u;if(welcome_done)*welcome_done=0u;
 for(uint32_t i=0;i<RU_VOL_SLOTS;i++){
  struct ru_vol_record rec;uint32_t off=base(bank)+RU_VOL_FIRST+i*RU_VOL_REC_BYTES;
  if(io->read(off,&rec,sizeof(rec)))return ETCH_STORE_IO;
  const uint8_t *bytes=(const uint8_t*)&rec;uint8_t blank=1u;
  for(uint32_t j=0;j<sizeof(rec);j++)if(bytes[j]!=255u){blank=0u;break;}
  if(blank && last_free==0xffffffffu)last_free=off;
  if(ru_vol_valid(&rec)){
   *value=rec.volume;
   if(autosave)*autosave=(ru_is_timed(rec.magic)||ru_is_boot_magic(rec.magic))?!!(rec.magic&2u):(uint8_t)(rec.magic==RU_SETTINGS_MAGIC||rec.magic==RU_BOTH_OFF_MAGIC);
   if(screensaver)*screensaver=(ru_is_timed(rec.magic)||ru_is_boot_magic(rec.magic))?!!(rec.magic&1u):(uint8_t)(rec.magic==RU_VOL_MAGIC||rec.magic==RU_SETTINGS_MAGIC);
   if(seconds_index)*seconds_index=ru_is_boot_magic(rec.magic)?(uint8_t)((ru_boot_payload(rec.magic)%32u)>>2):ru_is_timed(rec.magic)?(uint8_t)((rec.magic-RU_SAVER_TIMED_MAGIC)>>2):1u;
   if(boot_project)*boot_project=ru_is_boot_magic(rec.magic)?(uint8_t)(ru_boot_payload(rec.magic)/32u):0u;
   if(crt)*crt=(uint8_t)!ru_is_crt_off_magic(rec.magic);
   if(welcome_done)*welcome_done=ru_is_welcome_done_magic(rec.magic);
  }
 }
 if(free_offset)*free_offset=last_free;
 return ETCH_STORE_OK;
}
int etch_store_user_settings_welcome_load(const struct etch_store_io *io,
 uint8_t *volume,uint8_t *autosave,uint8_t *screensaver,uint8_t *seconds_index,uint8_t *boot_project,uint8_t *crt,uint8_t *welcome_done){
 if(!io||!io->read||!volume||!autosave||!screensaver||!seconds_index||!boot_project||!crt||!welcome_done)return ETCH_STORE_IO;
 *volume=100u;*autosave=0u;*screensaver=1u;*seconds_index=1u;*boot_project=0u;*crt=0u;*welcome_done=0u;
 int selected=select_project_read_region(io);if(selected<0)return selected;
 struct header h;int bank=current(io,&h,&scratch,&scratch_pattern,&scratch_settings);
 if(bank<0)return bank;
 if(h.version!=13u||store_region!=3u)return ETCH_STORE_OK;
 return ru_vol_scan(io,(uint32_t)bank,volume,autosave,screensaver,seconds_index,boot_project,0,crt,welcome_done);
}
int etch_store_user_settings_crt_load(const struct etch_store_io *io,uint8_t *volume,uint8_t *autosave,uint8_t *screensaver,uint8_t *seconds_index,uint8_t *boot_project,uint8_t *crt){
 uint8_t done=0u;return etch_store_user_settings_welcome_load(io,volume,autosave,screensaver,seconds_index,boot_project,crt,&done);
}
int etch_store_user_settings_full_load(const struct etch_store_io *io,uint8_t *volume,uint8_t *autosave,uint8_t *screensaver,uint8_t *seconds_index,uint8_t *boot_project){
 uint8_t crt=0u;return etch_store_user_settings_crt_load(io,volume,autosave,screensaver,seconds_index,boot_project,&crt);
}
int etch_store_user_settings_ex_load(const struct etch_store_io *io,uint8_t *volume,uint8_t *autosave,uint8_t *screensaver,uint8_t *seconds_index){
 uint8_t boot=0;return etch_store_user_settings_full_load(io,volume,autosave,screensaver,seconds_index,&boot);
}
int etch_store_user_settings_load(const struct etch_store_io *io,uint8_t *volume,uint8_t *autosave,uint8_t *screensaver){
 uint8_t delay=1u;return etch_store_user_settings_ex_load(io,volume,autosave,screensaver,&delay);
}
int etch_store_global_settings_load(const struct etch_store_io *io,uint8_t *volume,uint8_t *autosave){
 uint8_t saver=1u;return etch_store_user_settings_load(io,volume,autosave,&saver);
}
int etch_store_global_volume_load(const struct etch_store_io *io,uint8_t *volume){
 uint8_t autosave=0u;return etch_store_global_settings_load(io,volume,&autosave);
}
int etch_store_user_settings_welcome_save(const struct etch_store_io *io,uint8_t volume,uint8_t autosave,uint8_t screensaver,uint8_t seconds_index,uint8_t boot_project,uint8_t crt,uint8_t welcome_done){
 if(!io||!io->read||!io->program||volume>200u||autosave>1u||screensaver>1u||seconds_index>7u||boot_project>12u||crt>1u||welcome_done>1u)return ETCH_STORE_IO;
 int selected=select_project_read_region(io);if(selected<0)return selected;
 struct header h;int bank=current(io,&h,&scratch,&scratch_pattern,&scratch_settings);
 if(bank<0)return bank;
 if(h.version!=13u||store_region!=3u)return ETCH_STORE_INCOMPATIBLE;
 uint8_t previous,old_autosave,old_saver,old_seconds,old_boot,old_crt,old_done;uint32_t free_offset;
 int rc=ru_vol_scan(io,(uint32_t)bank,&previous,&old_autosave,&old_saver,&old_seconds,&old_boot,&free_offset,&old_crt,&old_done);if(rc)return rc;
 if(previous==volume&&old_autosave==autosave&&old_saver==screensaver&&old_seconds==seconds_index&&old_boot==boot_project&&old_crt==crt&&old_done==welcome_done)return ETCH_STORE_OK;
 if(free_offset==0xffffffffu){
  /* The 28-entry NOR journal is full. Compact by copying the complete,
   * previously verified project payload to the INACTIVE A/B bank. The
   * existing committed bank is untouched until the new header commits. */
  uint32_t target=(uint32_t)bank^1u,offset=base(target);
  for(uint32_t a=0;a<region_bank_bytes();a+=4096u)
   if(io->erase(offset+a))return ETCH_STORE_IO;
  uint8_t page[256];uint32_t crc=~0u;
  for(uint32_t a=0;a<h.bytes;a+=256u){
   uint32_t n=h.bytes-a;if(n>256u)n=256u;
   if(io->read(base((uint32_t)bank)+PAYLOAD_OFF+a,page,n))return ETCH_STORE_IO;
   if(io->program(offset+PAYLOAD_OFF+a,page,n))return ETCH_STORE_IO;
  }
  for(uint32_t a=0;a<h.bytes;a+=256u){
   uint32_t n=h.bytes-a;if(n>256u)n=256u;
   if(io->read(offset+PAYLOAD_OFF+a,page,n))return ETCH_STORE_IO;
   for(uint32_t j=0;j<n;j++){
    crc^=page[j];for(uint32_t k=0;k<8u;k++)
     crc=(crc>>1)^((0u-(crc&1u))&0xedb88320u);
   }
  }
  if(~crc!=h.crc)return ETCH_STORE_VERIFY;
  struct ru_vol_record compact=ru_vol_make(volume,autosave,screensaver,seconds_index,boot_project,crt,welcome_done);
  if(io->program(offset+RU_VOL_FIRST,&compact,sizeof(compact)))return ETCH_STORE_IO;
  struct ru_vol_record verify;
  if(io->read(offset+RU_VOL_FIRST,&verify,sizeof(verify))||
    !ru_vol_valid(&verify)||verify.volume!=volume||
    (uint8_t)!!(verify.magic&2u)!=autosave ||
    (uint8_t)!!(verify.magic&1u)!=screensaver ||
    (uint8_t)((ru_boot_payload(verify.magic)%32u)>>2)!=seconds_index ||
    (uint8_t)(ru_boot_payload(verify.magic)/32u)!=boot_project ||
    (uint8_t)!ru_is_crt_off_magic(verify.magic)!=crt ||
    ru_is_welcome_done_magic(verify.magic)!=welcome_done)return ETCH_STORE_VERIFY;
  h.bank=target;h.generation++;h.hcrc=etch_store_crc(&h,28u);
  if(io->program(offset,&h,sizeof(h)))return ETCH_STORE_IO;
  struct header committed;
  if(head(io,target,&committed)||committed.crc!=h.crc||
    committed.generation!=h.generation)return ETCH_STORE_VERIFY;
  return ETCH_STORE_OK;
 }
 struct ru_vol_record record=ru_vol_make(volume,autosave,screensaver,seconds_index,boot_project,crt,welcome_done),check;
 if(io->program(free_offset,&record,sizeof(record)))return ETCH_STORE_IO;
 if(io->read(free_offset,&check,sizeof(check)))return ETCH_STORE_IO;
 return ru_vol_valid(&check)&&check.volume==volume&&
  (uint8_t)!!(check.magic&2u)==autosave &&
  (uint8_t)!!(check.magic&1u)==screensaver &&
  (uint8_t)((ru_boot_payload(check.magic)%32u)>>2)==seconds_index &&
  (uint8_t)(ru_boot_payload(check.magic)/32u)==boot_project &&
  (uint8_t)!ru_is_crt_off_magic(check.magic)==crt &&
  ru_is_welcome_done_magic(check.magic)==welcome_done?ETCH_STORE_OK:ETCH_STORE_VERIFY;
}
int etch_store_user_settings_crt_save(const struct etch_store_io *io,uint8_t volume,uint8_t autosave,uint8_t screensaver,uint8_t seconds_index,uint8_t boot_project,uint8_t crt){
 uint8_t a=100u,b=0u,c=1u,d=1u,e=0u,old_crt=0u,done=0u;
 int rc=etch_store_user_settings_welcome_load(io,&a,&b,&c,&d,&e,&old_crt,&done);
 return rc==ETCH_STORE_OK?etch_store_user_settings_welcome_save(io,volume,autosave,screensaver,seconds_index,boot_project,crt,done):rc;
}
int etch_store_user_settings_full_save(const struct etch_store_io *io,uint8_t volume,uint8_t autosave,uint8_t screensaver,uint8_t seconds_index,uint8_t boot_project){
 uint8_t a=100u,b=0u,c=1u,d=1u,e=0u,crt=0u;
 int rc=etch_store_user_settings_crt_load(io,&a,&b,&c,&d,&e,&crt);
 return rc==ETCH_STORE_OK?etch_store_user_settings_crt_save(io,volume,autosave,screensaver,seconds_index,boot_project,crt):rc;
}
int etch_store_user_settings_ex_save(const struct etch_store_io *io,uint8_t volume,uint8_t autosave,uint8_t screensaver,uint8_t seconds_index){
 uint8_t a=0,b=0,c=0,d=0,boot=0;
 int rc=etch_store_user_settings_full_load(io,&a,&b,&c,&d,&boot);
 return rc==ETCH_STORE_OK?etch_store_user_settings_full_save(io,volume,autosave,screensaver,seconds_index,boot):rc;
}
int etch_store_user_settings_save(const struct etch_store_io *io,uint8_t volume,uint8_t autosave,uint8_t screensaver){
 uint8_t oldvol=100u,oldauto=0u,oldscreen=1u,oldseconds=1u;
 int rc=etch_store_user_settings_ex_load(io,&oldvol,&oldauto,&oldscreen,&oldseconds);
 return rc==ETCH_STORE_OK?etch_store_user_settings_ex_save(io,volume,autosave,screensaver,oldseconds):rc;
}
int etch_store_global_settings_save(const struct etch_store_io *io,uint8_t volume,uint8_t autosave){
 uint8_t oldvolume=100u,oldauto=0u,saver=1u;
 int rc=etch_store_user_settings_load(io,&oldvolume,&oldauto,&saver);
 return rc==ETCH_STORE_OK?etch_store_user_settings_save(io,volume,autosave,saver):rc;
}
int etch_store_global_volume_save(const struct etch_store_io *io,uint8_t volume){
 uint8_t remembered=100u,autosave=0u,saver=1u;
 int rc=etch_store_user_settings_load(io,&remembered,&autosave,&saver);
 return rc==ETCH_STORE_OK?etch_store_user_settings_save(io,volume,autosave,saver):rc;
}
static int erased(const struct etch_store_io *io,uint32_t off,uint32_t len){
 uint8_t page[256];
 while(len){uint32_t n=len>256?256:len;if(io->read(off,page,n))return ETCH_STORE_IO;for(uint32_t i=0;i<n;i++)if(page[i]!=255)return 0;off+=n;len-=n;}
 return 1;
}
/* Read-only probe: new records take precedence. On a blank expansion area,
 * legacy PJT9 snapshots remain readable, and the first new save migrates
 * without touching either old bank. Foreign data or torn uncommitted bytes
 * in the expanded region are NEVER erased automatically. */
static int select_project_read_region(const struct etch_store_io *io){
#if ETCH_STORE_OWNED_ENABLED
 struct header recognized;
 store_region=3u;
 int owned=current(io,&recognized,&scratch,&scratch_pattern,&scratch_settings);
 if(owned>=0)return 3;
 if(owned==ETCH_STORE_IO)return owned;
 store_region=0u;
 int legacy=current(io,&recognized,&scratch,&scratch_pattern,&scratch_settings);
 if(legacy>=0)return 0;
 if(legacy==ETCH_STORE_IO)return legacy;
#if ETCH_STORE_SPARE_ENABLED
 store_region=2u;
 int old_spare=current(io,&recognized,&scratch,&scratch_pattern,&scratch_settings);
 if(old_spare>=0)return 2;
 if(old_spare==ETCH_STORE_IO)return old_spare;
#endif
 /* The ETCH user banks are an independent storage target. An ERASED pair
  * can be claimed by the first normal SAVE, without looking at unrelated
  * legacy sample headers. Occupied unknown sectors still require an
  * explicit format gesture to avoid accidental data loss. */
 store_region=3u;
 return 3;
#elif ETCH_P04_EXPANDED
 /* Felucca sample slot 0 begins at 0xA0000: its header falls outside our
  * expanded bank, while its sample bytes extend INSIDE it. Reject that
  * owner marker even if the expanded banks appear blank. */
 uint32_t sample_magic=0;
 if(io->read(0xA0000u,&sample_magic,sizeof(sample_magic)))return ETCH_STORE_IO;
 if(sample_magic==0x504D5346u)return ETCH_STORE_FOREIGN; /* FSMP */
 struct header h;
 store_region=1;
 int r=current(io,&h,&scratch,&scratch_pattern,&scratch_settings);
 if(r>=0)return 1;
 if(r==ETCH_STORE_IO)return r;
 int clean=erased(io,ETCH_STORE_EXT_BASE,ETCH_STORE_EXT_END-ETCH_STORE_EXT_BASE);
 if(clean!=1)return clean==0?ETCH_STORE_FOREIGN:clean;
 store_region=0;
#else
 store_region=0;
#if ETCH_STORE_SPARE_ENABLED
 struct header old_h;
 int legacy=current(io,&old_h,&scratch,&scratch_pattern,&scratch_settings);
 if(legacy>=0)return 0; /* Preserve established ETCH banks. */
 if(legacy==ETCH_STORE_IO)return legacy;
 /* Felucca sample slot zero may own the entire subsequent sample pool.
  * Refuse even a blank-looking subregion when its FSMP header is present. */
 uint8_t sample_magic[4];
 if(io->read(0xA0000u,sample_magic,4u))return ETCH_STORE_IO;
 if(sample_magic[0]=='F'&&sample_magic[1]=='S'&&sample_magic[2]=='M'&&sample_magic[3]=='P'){
  store_region=0;return 0; /* preserve sample owner, report FOREIGN */
 }
 store_region=2;
 struct header spare_h;
 int spare=current(io,&spare_h,&scratch,&scratch_pattern,&scratch_settings);
 if(spare>=0)return 2;
 if(spare==ETCH_STORE_IO)return spare;
 int blank=erased(io,ETCH_STORE_SPARE_BASE,2u*ETCH_STORE_SPARE_BANK_BYTES);
 if(blank==1)return 2; /* Own these sectors only after full FF scan. */
 if(blank<0)return blank;
 store_region=0; /* Never modify unknown spare-area contents. */
#endif
#endif
 return 0;
}
static int select_project_write_region(const struct etch_store_io *io){
#if ETCH_STORE_OWNED_ENABLED
 int selection=select_project_read_region(io);
 if(selection<0)return selection;
 struct header recognized;
 int cur=current(io,&recognized,&scratch,&scratch_pattern,&scratch_settings);
 if(cur>=0)return cur;
 if(cur==ETCH_STORE_IO)return cur;
 if(store_region==3u){
  /* Fresh blank ETCH storage should work on the FIRST SAVE. Existing foreign
   * bytes cannot be erased through ordinary SAVE. */
  int blank=erased(io,ETCH_STORE_OWNED_BASE,ETCH_STORE_OWNED_END-ETCH_STORE_OWNED_BASE);
  return blank==1?-1:blank<0?blank:ETCH_STORE_FOREIGN;
 }
 return ETCH_STORE_FOREIGN;
#elif ETCH_P04_EXPANDED
 uint32_t sample_magic=0;
 if(io->read(0xA0000u,&sample_magic,sizeof(sample_magic)))return ETCH_STORE_IO;
 if(sample_magic==0x504D5346u)return ETCH_STORE_FOREIGN; /* Felucca FSMP */
 struct header h;
 store_region=1;
 int r=current(io,&h,&scratch,&scratch_pattern,&scratch_settings);
 if(r>=0)return r;
 if(r==ETCH_STORE_IO)return r;
 int clean=erased(io,ETCH_STORE_EXT_BASE,ETCH_STORE_EXT_END-ETCH_STORE_EXT_BASE);
 if(clean!=1)return clean==0?ETCH_STORE_FOREIGN:clean;
 return -1;
#else
#if ETCH_STORE_SPARE_ENABLED
 int region=select_project_read_region(io);
 if(region<0)return region;
 if(region==2){store_region=2;return -1;}
#endif
 store_region=0;return -1;
#endif
}
int etch_store_load(const struct etch_store_io *io,struct etch_wave_library *out){
 store_region=0;
 if(!io||!io->read||!out)return ETCH_STORE_IO;
 struct header h;int r=current(io,&h,&scratch,0,0);
 if(r>=0){uint8_t *d=(uint8_t*)out,*s=(uint8_t*)&scratch;for(uint32_t i=0;i<sizeof(*out);i++)d[i]=s[i];return ETCH_STORE_OK;}
 if(r==ETCH_STORE_IO)return r;
 r=erased(io,ETCH_STORE_BASE,2*ETCH_STORE_BANK_BYTES);return r==1?ETCH_STORE_EMPTY:r<0?r:ETCH_STORE_FOREIGN;
}
int etch_store_save(const struct etch_store_io *io,const struct etch_wave_library *src){
 store_region=0;
 if(!io||!io->read||!io->erase||!io->program||!src)return ETCH_STORE_IO;
 for(uint32_t w=0;w<3;w++)if(src->selection[w]>=64)return ETCH_STORE_VERIFY;
 struct header previous,h;int cur=current(io,&previous,&scratch,0,0);
 if(cur==ETCH_STORE_IO)return cur;
 if(cur>=0&&previous.version>=3)return ETCH_STORE_INCOMPATIBLE;
 if(cur<0){int r=erased(io,ETCH_STORE_BASE,2*ETCH_STORE_BANK_BYTES);if(r!=1)return r<0?r:ETCH_STORE_FOREIGN;}
 uint32_t to=cur==0?1:0,off=base(to);
 /* A valid snapshot owns the reserved pair. This permits retries after a
    torn erase/header, without mistaking partial ETCH payload for foreign data.
    A new pair is claimed only when every byte is erased. */
 for(uint32_t i=0;i<ETCH_STORE_BANK_BYTES;i+=4096)if(io->erase(off+i))return ETCH_STORE_IO;
 const uint8_t *data=(const uint8_t*)src;
 for(uint32_t i=0;i<sizeof(*src);i+=256){uint32_t n=sizeof(*src)-i;if(n>256)n=256;if(io->program(off+PAYLOAD_OFF+i,data+i,n))return ETCH_STORE_IO;}
 h.magic=MAGIC;h.version=2;h.generation=cur>=0?previous.generation+1:1;h.bytes=sizeof(*src);h.crc=etch_store_crc(src,sizeof(*src));h.bank=to;h.reserved=0;h.hcrc=etch_store_crc(&h,28);
 /* Read payload back before committing. */
 if(body(io,to,&h,&scratch,0,0))return ETCH_STORE_VERIFY;
 if(io->program(off,&h,32))return ETCH_STORE_IO;
 struct header verified;if(head(io,to,&verified)||body(io,to,&verified,&scratch,0,0))return ETCH_STORE_VERIFY;
 return ETCH_STORE_OK;
}

/* Explicit destructive initialization for foreign/torn first-claim data.
   Never called at boot or by ordinary SAVE. Does not erase a valid library. */
int etch_store_initialize(const struct etch_store_io *io,const struct etch_wave_library *src,uint32_t confirmation){
 store_region=0;
 if(confirmation!=ETCH_STORE_ERASE_CONFIRM||!io||!io->read||!io->erase||!io->program||!src)return ETCH_STORE_FOREIGN;
 for(uint32_t w=0;w<3;w++)if(src->selection[w]>=64)return ETCH_STORE_VERIFY;
 struct header previous;int cur=current(io,&previous,&scratch,0,0);
 if(cur>=0)return ETCH_STORE_FOREIGN;if(cur==ETCH_STORE_IO)return cur;
 for(uint32_t off=ETCH_STORE_BASE;off<ETCH_STORE_END;off+=4096)if(io->erase(off))return ETCH_STORE_IO;
 return etch_store_save(io,src);
}

int etch_store_patch_waves_ready(const struct etch_wave_library *l,const int16_t active[3][128]){
 if(!l||!active)return 0;
 for(uint32_t w=0;w<3;w++){
  uint32_t slot=l->selection[w];if(slot>=64)return 0;
  if(!(l->valid[slot>>5]&(1u<<(slot&31))))return 0;
  for(uint32_t i=0;i<128;i++)if(l->slot[slot][i]!=active[w][i])return 0;
 }
 return 1;
}

/* Compact V3 project commits use the same two unchanged flash banks,
 * with the old bank retained until the new CRC-verified commit header. */
int etch_store_load_project(const struct etch_store_io *io,struct etch_wave_library *out,
 struct etch_native_pattern *pat,struct etch_project_settings *settings){
 if(!io||!io->read||!out||!pat||!settings)return ETCH_STORE_IO;
 int selected=select_project_read_region(io);
 if(selected<0)return selected;
 struct header h;int r=current(io,&h,out,pat,settings);
 if(r>=0){
  if(h.version==11u||h.version==12u||h.version==13u){
   uint8_t selected=requested_slot<ETCH_P05_SLOTS?requested_slot:project_current_slot(h.reserved);
   if(requested_slot==255u)project_slot=selected;
   struct p05_manifest m;
   if(p05_read_catalog(io,(uint32_t)r,&h,&m)!=ETCH_STORE_OK||selected>=ETCH_P05_SLOTS)return ETCH_STORE_VERIFY;
   for(uint32_t j=0;j<12u;j++)project_name[j]=m.item[selected].name[j];
   /* Global waveform library: one authoritative project snapshot supplies
    * the wave slots/names/slot FX. An unrelated project's pattern, patches,
    * active slot selection and FX are not overwritten. The backup bank stays
    * valid until its replacement is verified and committed. */
   uint8_t owner=project_global_slot(h.reserved);
   if(owner<ETCH_P05_SLOTS && owner!=selected && out!=&scratch){
    uint8_t prior=requested_slot;requested_slot=owner;
    struct header global_head;
    int gr=current(io,&global_head,&scratch,&scratch_pattern,&scratch_settings);
    requested_slot=prior;
    if(gr<0)return ETCH_STORE_VERIFY;
    for(uint32_t j=0;j<sizeof(out->slot);j++)((uint8_t*)out->slot)[j]=((const uint8_t*)scratch.slot)[j];
    for(uint32_t j=0;j<sizeof(out->wave_names);j++)((uint8_t*)out->wave_names)[j]=((const uint8_t*)scratch.wave_names)[j];
    out->valid[0]=scratch.valid[0];out->valid[1]=scratch.valid[1];
    for(uint32_t j=0;j<sizeof(settings->slot_fx);j++)((uint8_t*)settings->slot_fx)[j]=((const uint8_t*)scratch_settings.slot_fx)[j];
    for(uint32_t w=0;w<3;w++)if(out->selection[w]<64u && (out->valid[out->selection[w]>>5]&(1u<<(out->selection[w]&31u))))
     for(uint32_t i=0;i<128;i++)out->active[w][i]=out->slot[out->selection[w]][i];
   }
  }
  return ETCH_STORE_OK;
 }
 if(r==ETCH_STORE_IO)return r;
#if ETCH_STORE_OWNED_ENABLED
 /* A blank ETCH-owned bank pair is ready for first project SAVE; foreign
  * data still needs a user-approved full-format operation. */
#endif
 r=erased(io,region_base(),2*region_bank_bytes());
 return r==1?ETCH_STORE_EMPTY:r<0?r:ETCH_STORE_FOREIGN;
}

/* V13 12-slot multi-project A/B snapshot, with v11/v12 read migration. Source records stream directly from
 * the previous committed bank: only the edited project's encoded bytes live
 * in RAM, so a second 64 KiB buffer is not required. All payloads and the
 * outer CRC are verified before the new commit header is written. */
static int p05_rewrite(const struct etch_store_io *io,const struct etch_wave_library *src,
 const struct etch_native_pattern *pat,const struct etch_project_settings *settings,int delete_slot){
 int r=select_project_write_region(io);
 if(r==ETCH_STORE_IO||r==ETCH_STORE_FOREIGN)return r;
 struct header prev={0},h={0};
 int cur=current(io,&prev,&scratch,&scratch_pattern,&scratch_settings);
 if(cur==ETCH_STORE_IO)return cur;
 if(cur<0){int blank=erased(io,region_base(),region_bank_bytes()*2u);
  if(blank!=1)return blank<0?blank:ETCH_STORE_FOREIGN;}
 /* Older ETCH used one project without a catalog. Its snapshot becomes
  * PROJECT 1, preserving its original data when saving a new project into
  * a later slot. Never reinterpret raw v1/v2 library bytes as a project. */
 if(cur>=0&&prev.version<3u&&project_slot!=0)return ETCH_STORE_INCOMPATIBLE;
 if(delete_slot&&(cur<0||(prev.version!=11&&prev.version!=12&&prev.version!=13)))return ETCH_STORE_EMPTY;
 /* Removing the global wave-library owner could make other project references
  * silently revert to unrelated waveforms. Refuse rather than corrupt them. */
 if(delete_slot&&project_slot==project_global_slot(prev.reserved))return ETCH_STORE_INCOMPATIBLE;
 struct p05_manifest old={0},next={0};
 if(cur>=0&&(prev.version==11||prev.version==12||prev.version==13)){
  if(p05_read_catalog(io,(uint32_t)cur,&prev,&old)!=ETCH_STORE_OK)return ETCH_STORE_VERIFY;
 }
 else if(cur>=0&&prev.version>=3u&&prev.version<=10u){
  /* Existing pre-P05 single-project image: retain it in slot zero. Its
   * existing payload and CRC are verified by current() before migration. */
  old.magic=ETCH_P05_V13_MAGIC;old.slots=ETCH_P05_SLOTS;
  old.item[0].offset=0u;old.item[0].bytes=prev.bytes;
  old.item[0].crc=prev.crc;
  {const char *title="LEGACY";
   for(uint32_t i=0;i<6u;i++)old.item[0].name[i]=title[i];}
 }
 uint32_t mine=0;
 if(delete_slot){
  if(!old.item[project_slot].bytes)return ETCH_STORE_EMPTY;
  uint8_t survivors=0;for(uint32_t i=0;i<ETCH_P05_SLOTS;i++)
   if(i!=project_slot&&old.item[i].bytes)survivors++;
  /* Keep at least one recoverable project; no empty catalog is bootable. */
  if(!survivors)return ETCH_STORE_INCOMPATIBLE;
 }else{
  int enc=etch_project_encode(src,pat,settings,packed,sizeof(packed),&mine);
  if(enc)return enc==-2?ETCH_STORE_FULL:ETCH_STORE_VERIFY;
 }
 next.magic=ETCH_P05_V13_MAGIC;next.slots=ETCH_P05_SLOTS;
 uint32_t total=sizeof(next);
 for(uint32_t i=0;i<ETCH_P05_SLOTS;i++){
  struct p05_item *item=&next.item[i];
  if(i==project_slot&&!delete_slot){item->bytes=mine;item->crc=etch_store_crc(packed,mine);
   for(uint32_t j=0;j<12u;j++)item->name[j]=project_name[j];}
  else if(i!=project_slot&&cur>=0)item->bytes=old.item[i].bytes,item->crc=old.item[i].crc;
  if(!item->bytes)continue;
  if(total>region_bank_bytes()-PAYLOAD_OFF||item->bytes>region_bank_bytes()-PAYLOAD_OFF-total)return ETCH_STORE_FULL;
  item->offset=total;total+=item->bytes;
  if(i!=project_slot&&cur>=0)for(uint32_t j=0;j<12u;j++)item->name[j]=old.item[i].name[j];
 }
 uint32_t to=cur==0?1u:0u,offset=base(to);
 for(uint32_t i=0;i<region_bank_bytes();i+=4096u){
  if(io->erase(offset+i))return ETCH_STORE_IO;
  ru_store_progress(1u,i+4096u,region_bank_bytes());
 }
 /* PJ52 manifest is 296 bytes. The FM1 SPI NOR HAL refuses any
  * single call over 256 bytes or crossing a 256-byte page boundary.
  * V1.12 accidentally wrote the entire manifest in one operation. */
 for(uint32_t at=0;at<sizeof(next);){
  uint32_t address=offset+PAYLOAD_OFF+at;
  uint32_t n=(uint32_t)sizeof(next)-at;
  uint32_t room=256u-(address&255u);if(n>room)n=room;
  if(io->program(address,(const uint8_t*)&next+at,n))return ETCH_STORE_IO;
  at+=n;ru_store_progress(2u,at,total);

 }
 uint8_t page[256];
 for(uint32_t slot=0;slot<ETCH_P05_SLOTS;slot++){
  const struct p05_item *item=&next.item[slot];if(!item->bytes)continue;
  for(uint32_t at=0;at<item->bytes;){
   uint32_t size=item->bytes-at;uint32_t room=256u-((offset+PAYLOAD_OFF+item->offset+at)&255u);if(size>room)size=room;
   if(slot==project_slot&&!delete_slot){for(uint32_t j=0;j<size;j++)page[j]=packed[at+j];}
   else if(io->read(base((uint32_t)cur)+PAYLOAD_OFF+old.item[slot].offset+at,page,size))return ETCH_STORE_IO;
   if(io->program(offset+PAYLOAD_OFF+item->offset+at,page,size))return ETCH_STORE_IO;
   at+=size;ru_store_progress(2u,item->offset+at,total);

  }
 }
 /* Read back and verify both the complete payload and each nested record. */
 uint32_t crc=~0u;
 for(uint32_t at=0;at<total;at+=256u){
  uint32_t size=total-at;if(size>256u)size=256u;
  if(io->read(offset+PAYLOAD_OFF+at,page,size))return ETCH_STORE_IO;
  ru_store_progress(3u,at+size,total);
  for(uint32_t j=0;j<size;j++){crc^=page[j];for(uint32_t k=0;k<8;k++)crc=(crc>>1)^((0u-(crc&1u))&0xEDB88320u);}
 }
 for(uint32_t slot=0;slot<ETCH_P05_SLOTS;slot++){
  const struct p05_item *item=&next.item[slot];if(!item->bytes)continue;
  uint32_t sub=~0u;
  for(uint32_t at=0;at<item->bytes;at+=256u){
   uint32_t size=item->bytes-at;if(size>256u)size=256u;
   if(io->read(offset+PAYLOAD_OFF+item->offset+at,page,size))return ETCH_STORE_IO;
   for(uint32_t j=0;j<size;j++){sub^=page[j];for(uint32_t k=0;k<8;k++)sub=(sub>>1)^((0u-(sub&1u))&0xEDB88320u);}
  }
  if(~sub!=item->crc)return ETCH_STORE_VERIFY;
 }
 uint8_t owner=global_wave_pending?project_slot:cur>=0?project_global_slot(prev.reserved):255u;
 h.magic=MAGIC;h.version=13u;h.generation=cur>=0?prev.generation+1u:1u;
 h.bytes=total;h.crc=~crc;h.bank=to;
 h.reserved=owner<ETCH_P05_SLOTS?(GLOBAL_OWNER_MAGIC|((uint32_t)owner<<8)|project_slot):project_slot;
 if(delete_slot){
  for(uint32_t i=0;i<ETCH_P05_SLOTS;i++)if(next.item[i].bytes){h.reserved=(h.reserved&~255u)|i;break;}
 }
 /* Carry the device-global volume into the new bank atomically with a
  * normal project save. Never copy torn/corrupted log entries. */
 if(cur>=0&&store_region==3u){
  uint8_t last_volume=100u,last_autosave=0u,last_saver=1u,last_seconds=1u,last_boot=0u,last_crt=0u,last_done=0u;
  int vr=ru_vol_scan(io,(uint32_t)cur,&last_volume,&last_autosave,&last_saver,&last_seconds,&last_boot,0,&last_crt,&last_done);if(vr)return vr;
  if(last_volume!=100u||last_autosave||!last_saver||last_seconds!=1u||last_boot||last_crt||last_done){
   struct ru_vol_record rec=ru_vol_make(last_volume,last_autosave,last_saver,last_seconds,last_boot,last_crt,last_done);
   if(io->program(offset+RU_VOL_FIRST,&rec,sizeof(rec)))return ETCH_STORE_IO;
  }
 }
 h.hcrc=etch_store_crc(&h,28u);
 if(io->program(offset,&h,sizeof(h)))return ETCH_STORE_IO;
 struct header verified;
 if(head(io,to,&verified)||verified.crc!=h.crc||verified.generation!=h.generation)return ETCH_STORE_VERIFY;
 global_wave_pending=0;
 return ETCH_STORE_OK;
}
/* Destructive catalog edit. Never deletes the final project; always commits to
 * the inactive bank after verifying the CRC of every remaining project. */
int etch_store_delete_project_slot(const struct etch_store_io *io,uint8_t slot,uint32_t confirmation){
 if(confirmation!=ETCH_STORE_ERASE_CONFIRM||slot>=ETCH_P05_SLOTS||
    !io||!io->read||!io->erase||!io->program)return ETCH_STORE_VERIFY;
 uint8_t old=project_slot;
 project_slot=slot;
 int result=p05_rewrite(io,0,0,0,1);
 project_slot=old;
 return result;
}

int etch_store_projects(const struct etch_store_io *io,uint16_t *occupied,char names[12][12]){
 if(!io||!io->read||!occupied||!names)return ETCH_STORE_IO;
 *occupied=0;for(uint32_t i=0;i<ETCH_P05_SLOTS;i++)for(uint32_t j=0;j<12;j++)names[i][j]=0;
 int selected=select_project_read_region(io);if(selected<0)return selected;
 struct header h;int bank=current(io,&h,&scratch,&scratch_pattern,&scratch_settings);
 if(bank<0)return bank;
 if(h.version==11||h.version==12||h.version==13){struct p05_manifest m;
  if(p05_read_catalog(io,(uint32_t)bank,&h,&m)!=ETCH_STORE_OK)return ETCH_STORE_VERIFY;
  for(uint32_t i=0;i<ETCH_P05_SLOTS;i++)if(m.item[i].bytes){*occupied|=(uint16_t)(1u<<i);
   for(uint32_t j=0;j<12;j++)names[i][j]=m.item[i].name[j];}
 }else{*occupied=1;const char *old="LEGACY";for(uint32_t j=0;old[j];j++)names[0][j]=old[j];}
 return ETCH_STORE_OK;
}
int etch_store_load_project_slot(const struct etch_store_io *io,uint8_t slot,
 struct etch_wave_library *lib,struct etch_native_pattern *pattern,struct etch_project_settings *cfg){
 if(slot>=ETCH_P05_SLOTS)return ETCH_STORE_VERIFY;
 /* Check the newest committed catalog BEFORE requesting a slot. An empty
  * slot must not silently fall back to a deleted project in the old bank. */
 uint16_t occupied=0;char names[12][12]={{0}};
 int catalog=etch_store_projects(io,&occupied,names);
 if(catalog!=ETCH_STORE_OK)return catalog;
 if(!(occupied & (1u<<slot)))return ETCH_STORE_EMPTY;
 etch_project_bind_preset_dsp(&scratch_settings,&scratch_dsp);
 requested_slot=slot;
 int result=etch_store_load_project(io,&scratch,&scratch_pattern,&scratch_settings);
 requested_slot=255;
 if(result!=ETCH_STORE_OK)return result;
 /* Separate output buffers must only be overwritten on a verified load. */
 const uint8_t *inputs[3]={(const uint8_t*)&scratch,(const uint8_t*)&scratch_pattern,(const uint8_t*)&scratch_settings};
 uint8_t *outputs[3]={(uint8_t*)lib,(uint8_t*)pattern,(uint8_t*)cfg};
 const uint32_t sizes[3]={sizeof(scratch),sizeof(scratch_pattern),sizeof(scratch_settings)};
 for(uint32_t k=0;k<3;k++)for(uint32_t j=0;j<sizes[k];j++)outputs[k][j]=inputs[k][j];
 etch_project_copy_preset_dsp(cfg,&scratch_settings);
 /* Load the authoritative global waves even when switching projects. */
 struct header h;int active=current(io,&h,&scratch,&scratch_pattern,&scratch_settings);
 if(active>=0 && (h.version==11u||h.version==12u||h.version==13u)){
  uint8_t owner=project_global_slot(h.reserved);
  if(owner<ETCH_P05_SLOTS && owner!=slot){
   requested_slot=owner;
   struct header gh;int rr=current(io,&gh,&scratch,&scratch_pattern,&scratch_settings);
   requested_slot=255;
   if(rr<0)return ETCH_STORE_VERIFY;
   for(uint32_t j=0;j<sizeof(lib->slot);j++)((uint8_t*)lib->slot)[j]=((const uint8_t*)scratch.slot)[j];
   for(uint32_t j=0;j<sizeof(lib->wave_names);j++)((uint8_t*)lib->wave_names)[j]=((const uint8_t*)scratch.wave_names)[j];
   lib->valid[0]=scratch.valid[0];lib->valid[1]=scratch.valid[1];
   for(uint32_t j=0;j<sizeof(cfg->slot_fx);j++)((uint8_t*)cfg->slot_fx)[j]=((const uint8_t*)scratch_settings.slot_fx)[j];
   for(uint32_t w=0;w<3;w++)if(lib->selection[w]<64u && (lib->valid[lib->selection[w]>>5]&(1u<<(lib->selection[w]&31u))))
    for(uint32_t i=0;i<128;i++)lib->active[w][i]=lib->slot[lib->selection[w]][i];
  }
 }
 project_slot=slot;return ETCH_STORE_OK;
}
void etch_store_project_name(const char *name){
 uint8_t ended=0;
 for(uint32_t j=0;j<12;j++){if(!name||ended||!name[j]){ended=1;project_name[j]=0;}else project_name[j]=name[j];}
}
int etch_store_save_project(const struct etch_store_io *io,const struct etch_wave_library *src,
 const struct etch_native_pattern *pat,const struct etch_project_settings *settings){
 if(!io||!io->read||!io->erase||!io->program||!src||!pat||!settings)return ETCH_STORE_IO;
#if ETCH_P05_MULTI
 return p05_rewrite(io,src,pat,settings,0);
#endif
 int select=select_project_write_region(io);
 if(select==ETCH_STORE_IO||select==ETCH_STORE_FOREIGN)return select;
 struct header previous,h;int cur=current(io,&previous,&scratch,&scratch_pattern,&scratch_settings);
 if(cur==ETCH_STORE_IO)return cur;
 if(cur<0){int r=erased(io,region_base(),2*region_bank_bytes());if(r!=1)return r<0?r:ETCH_STORE_FOREIGN;}
 uint32_t n=0;int rc=etch_project_encode(src,pat,settings,packed,sizeof(packed),&n);
 if(rc)return rc==-2?ETCH_STORE_FULL:ETCH_STORE_VERIFY;
 uint32_t to=cur==0?1:0,off=base(to);
 for(uint32_t i=0;i<region_bank_bytes();i+=4096)if(io->erase(off+i))return ETCH_STORE_IO;
 for(uint32_t i=0;i<n;i+=256){uint32_t k=n-i;if(k>256)k=256;if(io->program(off+PAYLOAD_OFF+i,packed+i,k))return ETCH_STORE_IO;}
 h.magic=MAGIC;h.version=ETCH_P04_EXPANDED?10:9;h.generation=cur>=0?previous.generation+1:1;
 h.bytes=n;h.crc=etch_store_crc(packed,n);h.bank=to;h.reserved=0;h.hcrc=etch_store_crc(&h,28);
 /* CRC verify raw bytes without decoding into caller-owned live memory. */
 uint8_t page[256];uint32_t c=~0u;
 for(uint32_t i=0;i<n;i+=256){uint32_t k=n-i;if(k>256)k=256;
  if(io->read(off+PAYLOAD_OFF+i,page,k))return ETCH_STORE_IO;
  for(uint32_t j=0;j<k;j++){c^=page[j];for(uint32_t b=0;b<8;b++)c=(c>>1)^((0u-(c&1u))&0xedb88320u);}
 }
 if(~c!=h.crc)return ETCH_STORE_VERIFY;
 if(io->program(off,&h,32))return ETCH_STORE_IO;
 struct header verified;if(head(io,to,&verified))return ETCH_STORE_VERIFY;
 if(verified.generation!=h.generation||verified.bytes!=n||verified.crc!=h.crc)return ETCH_STORE_VERIFY;
 return ETCH_STORE_OK;
}
int etch_store_initialize_project(const struct etch_store_io *io,const struct etch_wave_library *lib,
 const struct etch_native_pattern *pat,const struct etch_project_settings *cfg,uint32_t confirm){
 if(confirm!=ETCH_STORE_ERASE_CONFIRM||!io||!io->read||!io->erase||!io->program||!lib||!pat||!cfg)
  return ETCH_STORE_FOREIGN;
#if ETCH_STORE_OWNED_ENABLED
 /* First-time confirmed ETCH user-storage format. The owned project/wave
  * bank pair is 0xB4000..0xDC000: keep ALL application, OTA, boot and other
  * unrelated regions outside the write range. Existing valid ETCH projects
  * cannot be erased through a repeated format gesture. */
 struct header h;store_region=3u;
 int existing=current(io,&h,&scratch,&scratch_pattern,&scratch_settings);
 if(existing>=0)return ETCH_STORE_INCOMPATIBLE;
 if(existing==ETCH_STORE_IO)return existing;
 uint32_t n=0;
 int measured=etch_project_encode(lib,pat,cfg,packed,sizeof(packed),&n);
 if(measured)return measured==-2?ETCH_STORE_FULL:ETCH_STORE_VERIFY;
 /* Prevent silently abandoning valid legacy projects; user must export them
  * before changing storage ownership. */
 store_region=0u;
 existing=current(io,&h,&scratch,&scratch_pattern,&scratch_settings);
 if(existing>=0)return ETCH_STORE_INCOMPATIBLE;
 if(existing==ETCH_STORE_IO)return existing;
#if ETCH_STORE_SPARE_ENABLED
 store_region=2u;
 existing=current(io,&h,&scratch,&scratch_pattern,&scratch_settings);
 if(existing>=0)return ETCH_STORE_INCOMPATIBLE;
 if(existing==ETCH_STORE_IO)return existing;
#endif
 for(uint32_t off=ETCH_STORE_OWNED_BASE;off<ETCH_STORE_OWNED_END;off+=4096u)
  if(io->erase(off))return ETCH_STORE_IO;
 store_region=3u;
 store_takeover_active=1u;
 int result=etch_store_save_project(io,lib,pat,cfg);
 store_takeover_active=0u;
 return result;
#else
 struct header h;store_region=0u;
 int cur=current(io,&h,&scratch,&scratch_pattern,&scratch_settings);
 if(cur>=0)return ETCH_STORE_INCOMPATIBLE;
 if(cur==ETCH_STORE_IO)return cur;
 uint32_t n=0;int rc=etch_project_encode(lib,pat,cfg,packed,sizeof(packed),&n);
 if(rc)return rc==-2?ETCH_STORE_FULL:ETCH_STORE_VERIFY;
 for(uint32_t off=ETCH_STORE_BASE;off<ETCH_STORE_END;off+=4096u)if(io->erase(off))return ETCH_STORE_IO;
 return etch_store_save_project(io,lib,pat,cfg);
#endif
}


/* RUPTURE V1.28: metadata-only catalog edits. Stream complete CRC-checked
 * project records into the inactive bank; the running project is untouched. */
int etch_store_catalog_edit(const struct etch_store_io *io,uint8_t op,uint8_t source,
 uint8_t target,const char *rename,uint32_t confirmation){
 if(!io||!io->read||!io->erase||!io->program||source>=12u||target>=12u||
    (op!=1u&&op!=2u&&op!=3u)||confirmation!=ETCH_STORE_ERASE_CONFIRM)return ETCH_STORE_VERIFY;
 int r=select_project_write_region(io);
 if(r<0)return r;
 struct header oldhead={0},hnew={0};
 int cur=current(io,&oldhead,&scratch,&scratch_pattern,&scratch_settings);
 if(cur<0)return cur;
 if(oldhead.version!=13u)return ETCH_STORE_INCOMPATIBLE;
 struct p05_manifest old={0},next={0};
 if(p05_read_catalog(io,(uint32_t)cur,&oldhead,&old)!=ETCH_STORE_OK)return ETCH_STORE_VERIFY;
 if(!old.item[source].bytes)return ETCH_STORE_EMPTY;
 if(op==3u){
  uint8_t survivors=0;
  for(uint8_t i=0;i<12u;i++)if(i!=source&&old.item[i].bytes)survivors++;
  if(!survivors){
   /* Last project: erase the dedicated OWNED bank pair only, retaining other
    * firmware, OTA, boot and legacy records. This is an explicit delete. */
   if(store_region!=3u)return ETCH_STORE_INCOMPATIBLE;
   for(uint32_t bank=0;bank<2u;bank++)for(uint32_t at=0;at<region_bank_bytes();at+=4096u){
    if(io->erase(base(bank)+at))return ETCH_STORE_IO;
    ru_store_progress(1u,bank*region_bank_bytes()+at+4096u,2u*region_bank_bytes());
   }
   project_slot=0u;requested_slot=255u;global_wave_pending=0u;
   return ETCH_STORE_OK;
  }
 }
 if(op==1u && (old.item[target].bytes||source==target))return ETCH_STORE_INCOMPATIBLE;
 if(op==2u && (!rename||!rename[0]))return ETCH_STORE_VERIFY;
 if(op==3u){
  /* Global wave-library ownership cannot simply disappear and corrupt
   * surviving project references. Reject this special case, not data-loss. */
  /* Shared waveform owner cannot be deleted while other projects depend
   * on it. Deleting the sole project uses the dedicated final-slot path above. */
  if(source==project_global_slot(oldhead.reserved))return ETCH_STORE_SHARED_OWNER;
 }
 next.magic=ETCH_P05_V13_MAGIC;next.slots=ETCH_P05_SLOTS;
 uint32_t total=sizeof(next);
 for(uint8_t i=0;i<12u;i++){
  struct p05_item *item=&next.item[i];
  const struct p05_item *original=&old.item[(op==1u && i==target)?source:i];
  if(op==3u&&i==source)continue;
  if(!original->bytes)continue;
  item->bytes=original->bytes;item->crc=original->crc;
  for(uint8_t j=0;j<12u;j++)item->name[j]=original->name[j];
  if(op==2u&&i==source){
   for(uint8_t j=0;j<12u;j++)item->name[j]=rename[j]?rename[j]:' ';
  }
  if(total>region_bank_bytes()-PAYLOAD_OFF||item->bytes>region_bank_bytes()-PAYLOAD_OFF-total)return ETCH_STORE_FULL;
  item->offset=total;total+=item->bytes;
 }
 uint32_t destination=(uint32_t)cur^1u,offset=base(destination);
 for(uint32_t a=0;a<region_bank_bytes();a+=4096u){
  if(io->erase(offset+a))return ETCH_STORE_IO;
  ru_store_progress(1u,a+4096u,region_bank_bytes());
 }
 uint8_t page[256];
 for(uint32_t a=0;a<sizeof(next);){
  uint32_t pos=offset+PAYLOAD_OFF+a,n=sizeof(next)-a;
  uint32_t room=256u-(pos&255u);if(n>room)n=room;
  if(io->program(pos,(const uint8_t*)&next+a,n))return ETCH_STORE_IO;
  a+=n;
 }
 for(uint8_t slot=0;slot<12u;slot++)if(next.item[slot].bytes){
  const struct p05_item *item=&next.item[slot];
  const struct p05_item *original=&old.item[(op==1u&&slot==target)?source:slot];
  for(uint32_t a=0;a<item->bytes;){
   uint32_t addr=offset+PAYLOAD_OFF+item->offset+a,n=item->bytes-a;
   uint32_t room=256u-(addr&255u);if(n>room)n=room;
   if(io->read(base((uint32_t)cur)+PAYLOAD_OFF+original->offset+a,page,n) ||
       io->program(addr,page,n))return ETCH_STORE_IO;
   a+=n;ru_store_progress(2u,item->offset+a,total);
  }
 }
 uint32_t crc=~0u;
 for(uint32_t a=0;a<total;a+=256u){
  uint32_t n=total-a;if(n>256u)n=256u;
  if(io->read(offset+PAYLOAD_OFF+a,page,n))return ETCH_STORE_IO;
  for(uint32_t j=0;j<n;j++){crc^=page[j];for(uint32_t k=0;k<8;k++)crc=(crc>>1)^((0u-(crc&1u))&0xedb88320u);}
  ru_store_progress(3u,a+n,total);
 }
 for(uint8_t slot=0;slot<12u;slot++)if(next.item[slot].bytes){
  const struct p05_item *item=&next.item[slot];uint32_t check=~0u;
  for(uint32_t a=0;a<item->bytes;a+=256u){
   uint32_t n=item->bytes-a;if(n>256u)n=256u;
   if(io->read(offset+PAYLOAD_OFF+item->offset+a,page,n))return ETCH_STORE_IO;
   for(uint32_t j=0;j<n;j++){check^=page[j];for(uint32_t k=0;k<8;k++)check=(check>>1)^((0u-(check&1u))&0xedb88320u);}
  }
  if(~check!=item->crc)return ETCH_STORE_VERIFY;
 }
 if(store_region==3u){
  uint8_t vol=100u,autosave=0u,saver=1u,seconds=1u,boot=0u,crt=0u,done=0u;
  int gr=ru_vol_scan(io,(uint32_t)cur,&vol,&autosave,&saver,&seconds,&boot,0,&crt,&done);if(gr)return gr;
  if(vol!=100u||autosave||!saver||seconds!=1u||boot||crt||done){
   struct ru_vol_record rec=ru_vol_make(vol,autosave,saver,seconds,boot,crt,done);
   if(io->program(offset+RU_VOL_FIRST,&rec,sizeof(rec)))return ETCH_STORE_IO;
  }
 }
 hnew.magic=MAGIC;hnew.version=13u;hnew.generation=oldhead.generation+1u;
 hnew.bytes=total;hnew.crc=~crc;hnew.bank=destination;hnew.reserved=oldhead.reserved;
 if(op==3u && project_current_slot(hnew.reserved)==source){
  for(uint8_t i=0;i<12u;i++)if(next.item[i].bytes){hnew.reserved=(hnew.reserved&~255u)|i;break;}
 }
 hnew.hcrc=etch_store_crc(&hnew,28u);
 if(io->program(offset,&hnew,sizeof(hnew)))return ETCH_STORE_IO;
 struct header verify;
 if(head(io,destination,&verify)||verify.generation!=hnew.generation||verify.crc!=hnew.crc)return ETCH_STORE_VERIFY;
 return ETCH_STORE_OK;
}
/* Legacy ETCH factory labels are placeholders only when no pattern data
 * exists. Never hide a populated sequence merely due to its name. */
static uint8_t ru_store_legacy_sequence_name(uint8_t slot,const char *name){
 if(slot<24u||slot>31u||!name)return 0u;
 if(name[0]!='E'||name[1]!='T'||name[2]!='C'||name[3]!='H')return 0u;
 uint8_t digit=(uint8_t)(name[4]==' '?5u:4u);
 if(name[digit]!=(char)('0'+slot/10u)||name[digit+1u]!=(char)('0'+slot%10u))return 0u;
 for(uint8_t j=(uint8_t)(digit+2u);j<12u;j++)if(name[j] && name[j]!=' ')return 0u;
 return 1u;
}
/* Read-only list of the eight sequences belonging to ANY stored project.
 * Never changes the project currently selected for playing. */
int etch_store_project_sequence_list(const struct etch_store_io *io,uint8_t project,
 uint8_t *mask,char names[8][13]){
 if(!io||!mask||!names||project>=12u)return ETCH_STORE_IO;
 *mask=0u;for(uint8_t i=0;i<8u;i++)for(uint8_t j=0;j<13u;j++)names[i][j]=0;
 uint8_t previous=requested_slot;requested_slot=project;
 struct header h={0};int bank=current(io,&h,&scratch,&scratch_pattern,&scratch_settings);
 requested_slot=previous;
 if(bank<0)return bank;
 for(uint8_t i=0;i<8u;i++){
  uint8_t slot=(uint8_t)(24u+i);
  uint8_t used=(uint8_t)(scratch_settings.patterns.lengths[slot] ||
   (scratch.patches[slot].valid && !ru_store_legacy_sequence_name(slot,scratch.patches[slot].name)));
  if(used){
   *mask|=(uint8_t)(1u<<i);
   for(uint8_t j=0;j<12u;j++)names[i][j]=scratch.patches[slot].name[j];
  }
 }
 return ETCH_STORE_OK;
}
/* The documented FM1 storage map: 0x97000..0xDFFFF contains legacy
 * ETCH project banks and Felucca samples/presets. Never touch 0xE0000..
 * (OTA staging), 0x93000..0x96FFF (OTA loader), or the application. */
#define RUPTURE_STORAGE_WIPE_END 0xE0000u
_Static_assert(ETCH_STORE_OWNED_END<=RUPTURE_STORAGE_WIPE_END,"Wipe must remain in user storage");
int etch_store_wipe_all_user_data(const struct etch_store_io *io,uint32_t confirm,
 uint32_t second_confirm){
 if(!io||!io->erase||!io->read||confirm!=ETCH_STORE_ERASE_CONFIRM||
    second_confirm!=0x52555054u)return ETCH_STORE_VERIFY;
 for(uint32_t at=ETCH_STORE_BASE;at<RUPTURE_STORAGE_WIPE_END;at+=4096u){
  if(io->erase(at))return ETCH_STORE_IO;
  ru_store_progress(1u,at-ETCH_STORE_BASE+4096u,RUPTURE_STORAGE_WIPE_END-ETCH_STORE_BASE);
 }
 uint8_t bytes[256];
 for(uint32_t at=ETCH_STORE_BASE;at<RUPTURE_STORAGE_WIPE_END;at+=256u){
  if(io->read(at,bytes,sizeof(bytes)))return ETCH_STORE_IO;
  for(uint32_t i=0;i<sizeof(bytes);i++)if(bytes[i]!=255u)return ETCH_STORE_VERIFY;
  ru_store_progress(3u,at-ETCH_STORE_BASE+256u,RUPTURE_STORAGE_WIPE_END-ETCH_STORE_BASE);
 }
 project_slot=0u;requested_slot=255u;global_wave_pending=0u;store_region=3u;
 return ETCH_STORE_OK;
}
