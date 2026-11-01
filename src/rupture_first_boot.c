/* SPDX-License-Identifier: GPL-3.0-only */
#include "rupture_first_boot.h"
#include "rupture_data.h"
RuptureStorageKind rupture_classify_header(const uint8_t *data,uint32_t length){
 if(!data||length<16)return RUPTURE_STORAGE_FOREIGN;
 int erased=1;for(uint32_t i=0;i<length;i++)if(data[i]!=0xffu){erased=0;break;}
 if(erased)return RUPTURE_STORAGE_EMPTY;
 uint32_t magic=(uint32_t)data[0]|((uint32_t)data[1]<<8)|((uint32_t)data[2]<<16)|((uint32_t)data[3]<<24);
 uint32_t version=(uint32_t)data[4]|((uint32_t)data[5]<<8)|((uint32_t)data[6]<<16)|((uint32_t)data[7]<<24);
 if(magic==RUPTURE_STORAGE_MAGIC && version==RUPTURE_STORAGE_VERSION &&
    length >= sizeof(RuptureLibrary) && rupture_library_valid((const RuptureLibrary*)data))
   return RUPTURE_STORAGE_OWNED;
 return RUPTURE_STORAGE_FOREIGN;
}
int rupture_format_authorized(int initial,int confirmed,int whitelist,int overlap){return initial&&confirmed&&whitelist&&!overlap;}
