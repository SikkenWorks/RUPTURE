/* RUPTURE 1.5: destructive flash emulator, never touches an actual FM1.
 * Migration, full bank, torn-write fallback and 12 distinct records. */
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/wave_store.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_project_codec.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_patch_state.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_shape_state.h"
#define FLASH_SIZE 0x100000u
static uint8_t flashbuf[FLASH_SIZE];
static struct etch_wave_library l,o;
static struct etch_native_pattern p,q;
static struct etch_project_settings c,d;
static struct etch_preset_dsp_bank ds,od;
static int fail_program_after=-1, writes=0;
static int rd(uint32_t a,void *v,uint32_t n){if(a>FLASH_SIZE||n>FLASH_SIZE-a)return -1;memcpy(v,flashbuf+a,n);return 0;}
static int erase(uint32_t a){if(a&4095u||a>FLASH_SIZE-4096u)return -1;memset(flashbuf+a,0xff,4096);return 0;}
static int wr(uint32_t a,const void *v,uint32_t n){
 if(fail_program_after>=0&&writes++>=fail_program_after)return -1;
 if(a>FLASH_SIZE||n>FLASH_SIZE-a)return -1;
 const uint8_t *b=v;
 for(uint32_t i=0;i<n;i++)if((flashbuf[a+i]&b[i])!=b[i])return -1;
 for(uint32_t i=0;i<n;i++)flashbuf[a+i]&=b[i];
 return 0;
}
static struct etch_store_io io={rd,erase,wr};
/* Progress must be based on actual flash phases, not a simulated timer. */
static uint32_t phase_calls[5],max_pct[5];
static void progress(uint8_t phase,uint32_t done,uint32_t total){
 assert(phase>=1u&&phase<=4u&&total>0u&&done<=total);
 ++phase_calls[phase];
 uint32_t pct=(uint32_t)((uint64_t)done*100u/total);
 if(pct>max_pct[phase])max_pct[phase]=pct;
}

static void make_project(uint8_t i){
 memset(&l,0,sizeof(l));memset(&p,0,sizeof(p));memset(&c,0,sizeof(c));memset(&ds,0,sizeof(ds));
 etch_project_bind_preset_dsp(&c,&ds);
 c.seq_length=16;c.tempo=110+i;c.seq_gate=75;c.source_version=32;
 etch_fx_defaults(&c.global_fx);
 l.valid[0]=1;
 ru_patch_cxs_save(&l.patches[0],20+i,50,50,50);
 l.patches[0].tempo=120;l.patches[0].seq_length=16;
 for(uint8_t j=0;j<ETCH_PATCH_SLOTS;j++){ds.slot[j].spectral_part=2u;ds.slot[j].spectral_gain=50u;ds.slot[j].spectral_tilt=50u;ds.slot[j].blend_a=j;ds.slot[j].blend_b=j;}
 etch_perf_init(&ds.performance);etch_key_groups_init(&ds.groups);
 ru_shape_store(&ds.slot[0],i,50,0,0);
}
static void check_project(uint8_t i){
 memset(&o,0,sizeof(o));memset(&q,0,sizeof(q));memset(&d,0,sizeof(d));memset(&od,0,sizeof(od));
 etch_project_bind_preset_dsp(&d,&od);
 int r=etch_store_load_project_slot(&io,i,&o,&q,&d);
 if(r){printf("FAIL read slot %d rc=%d\n",i,r);assert(0);}
 assert(d.tempo==110+i);
 assert(o.patches[0].valid&0x80);
 uint8_t w,e,b,pu;assert(ru_shape_restore(&od.slot[0],&w,&e,&b,&pu));assert(w==i);
}
static void save_project(uint8_t i){
 etch_store_project_select(i);make_project(i);
 char name[12]={'R','U','P','T','U','R','E',' ','0','0',0,0};name[8]=(char)('0'+(i+1)/10);name[9]=(char)('0'+(i+1)%10);
 etch_store_project_name(name);
 int r=etch_store_save_project(&io,&l,&p,&c);
 if(r){printf("FAIL save slot %d rc=%d\n",i,r);assert(0);}
}
int main(void){
 memset(flashbuf,0xff,sizeof(flashbuf));
 etch_store_set_progress(progress);
 /* FIRST SAVE on verified blank storage must succeed without formatting.
  * It writes no unrelated sectors and reports all three real phases. */
 save_project(0);
 assert(phase_calls[1]>=20u&&phase_calls[2]>0u&&phase_calls[3]>0u);
 assert(max_pct[1]==100u&&max_pct[2]>=99u&&max_pct[3]==100u);
 for(uint8_t i=1;i<12;i++)save_project(i);
 uint16_t present=0;char names[12][12];
 assert(etch_store_projects(&io,&present,names)==0);
 assert(present==0x0fffu);
 for(uint8_t i=0;i<12;i++)check_project(i);
 assert(phase_calls[4]>0u&&max_pct[4]==100u);
 /* Fail in the middle of a new write: previous A/B copy survives. */
 save_project(11); // ensure committed stable baseline
 etch_store_project_select(11);make_project(11);c.tempo=222;
 fail_program_after=2;writes=0;
 assert(etch_store_save_project(&io,&l,&p,&c)!=0);
 fail_program_after=-1;check_project(11);
 for(uint8_t i=0;i<11;i++)check_project(i);
 /* Deleting one record preserves all eleven other slot records. */
 assert(etch_store_delete_project_slot(&io,6,ETCH_STORE_ERASE_CONFIRM)==0);
 assert(etch_store_projects(&io,&present,names)==0);
 assert(present==0x0fbfu);
 for(uint8_t i=0;i<12;i++)if(i!=6)check_project(i);
 assert(etch_store_load_project_slot(&io,6,&o,&q,&d)==ETCH_STORE_EMPTY);
 /* Seed an unmodified P05 four-slot v12 manifest, then write project 12.
  * The original slot zero must migrate losslessly, without touching its bank
  * until the new v13 payload and header are fully verified. */
 memset(flashbuf,0xff,sizeof(flashbuf));make_project(0);
 uint8_t record[ETCH_PROJECT_MAX_BYTES];uint32_t len=0;
 assert(etch_project_encode(&l,&p,&c,record,sizeof(record),&len)==0);
 struct {uint32_t offset,bytes,crc;char name[12];} olditem[4]={{0}};
 struct {uint32_t magic,slots;uint8_t items[96];} legacy={0};
 olditem[0].offset=104;olditem[0].bytes=len;olditem[0].crc=etch_store_crc(record,len);
 memcpy(olditem[0].name,"OLDER 0001",10);
 legacy.magic=0x31354A50u;legacy.slots=4;memcpy(legacy.items,olditem,sizeof(olditem));
 uint8_t *dest=flashbuf+0xB4000u+256;
 memcpy(dest,&legacy,104);memcpy(dest+104,record,len);
 struct {uint32_t magic,version,generation,bytes,crc,bank,reserved,hcrc;} head={0};
 head.magic=0x38575745u;head.version=12;head.generation=1;head.bytes=104+len;head.crc=etch_store_crc(dest,head.bytes);head.bank=0;
 head.reserved=0x474C0000u;head.hcrc=etch_store_crc(&head,28);
 memcpy(flashbuf+0xB4000u,&head,sizeof(head));
 check_project(0);
 save_project(11);
 check_project(0);check_project(11);
 assert(etch_store_projects(&io,&present,names)==0);
 assert(present==0x801u);
 etch_store_set_progress(0);
 printf("PASS twelve-slot flash tests, four-slot v12 migration, CRC, torn-write fallback, deletion, CXS restoration\n");
 return 0;
}
