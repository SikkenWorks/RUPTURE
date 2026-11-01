/* SPDX-License-Identifier: GPL-3.0-only */
#include "rupture_data.h"
#include <stddef.h>
uint32_t rupture_crc32(const void *data,uint32_t size){const unsigned char *p=(const unsigned char*)data;uint32_t c=0xffffffffu;
 for(uint32_t i=0;i<size;i++){c^=p[i];for(unsigned bit=0;bit<8;bit++)c=(c>>1)^((c&1u)?0xedb88320u:0u);}return ~c;}
void rupture_library_seal(RuptureLibrary *db){db->crc=rupture_crc32(&db->project[0],(uint32_t)sizeof(db->project));}
void rupture_library_init(RuptureLibrary *db){{uint8_t *data=(uint8_t*)db;for(uint32_t i=0;i<(uint32_t)sizeof(*db);i++)data[i]=0;}db->magic=RUPTURE_STORAGE_MAGIC;db->version=RUPTURE_STORAGE_VERSION;
 for(int p=0;p<RUPTURE_PROJECTS;p++){db->project[p].name[0]='P';db->project[p].name[1]='0'+(p+1)/10;db->project[p].name[2]='0'+(p+1)%10;db->project[p].name[3]='\0';db->project[p].root=0;db->project[p].scale=0;db->project[p].keylock=0;db->project[p].guide=1;
 for(int s=0;s<RUPTURE_SEQ_SLOTS;s++){RuptureSequence *q=&db->project[p].sequence[s];q->length=16;q->rate=4;q->gate=75;for(int n=0;n<RUPTURE_STEPS;n++){q->step[n].note=255;q->step[n].velocity=100;q->step[n].length=1;}}
 }rupture_library_seal(db);}
int rupture_library_valid(const RuptureLibrary *db){return db && db->magic==RUPTURE_STORAGE_MAGIC&&db->version==RUPTURE_STORAGE_VERSION&&db->crc==rupture_crc32(db->project,(uint32_t)sizeof(db->project));}
