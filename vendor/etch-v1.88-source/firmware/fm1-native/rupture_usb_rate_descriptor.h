/* SPDX-License-Identifier: GPL-3.0-only
 * Pure USB Audio Class 1.0 Type-I rate lock for a two-frequency descriptor.
 * Does not alter descriptor size or any unrelated interface/endpoint.
 */
#pragma once
#include <stdint.h>

static inline uint8_t ru_uac_restrict_format(uint8_t *cfg,uint32_t bytes,uint8_t rate48)
{
 uint32_t hz=rate48?48000u:44100u;
 for(uint32_t at=0u;at<bytes;){
  uint32_t n=cfg[at];
  if(!n||at+n>bytes)break;
  if(n==14u&&cfg[at+1u]==0x24u&&cfg[at+2u]==2u&&
     cfg[at+3u]==1u&&cfg[at+7u]<=2u){
   cfg[at+7u]=0u; /* bSamFreqType=0: continuous min/max range */
   for(uint32_t i=0u;i<3u;i++){
    uint8_t octet=(uint8_t)(hz>>(i*8u));
    cfg[at+8u+i]=octet;
    cfg[at+11u+i]=octet;
   }
   return 1u;
  }
  at+=n;
 }
 return 0u;
}
