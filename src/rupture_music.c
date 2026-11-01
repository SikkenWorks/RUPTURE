/* SPDX-License-Identifier: GPL-3.0-only */
#include "rupture_music.h"
static const uint16_t scale_mask[6]={0x0fffu,0x0ab5u,0x05adu,0x06adu,0x0295u,0x04e9u};
static uint8_t norm_root(uint8_t n){return n%12u;}
int rupture_in_scale(uint8_t note,uint8_t root,uint8_t scale){
 if(scale>=6u)return 0;
 uint8_t delta=(uint8_t)((note%12u+12u-norm_root(root))%12u);
 return (scale_mask[scale]&(1u<<delta))!=0;
}
uint8_t rupture_keylock(uint8_t note,uint8_t root,uint8_t scale){
 if(scale>=6u||rupture_in_scale(note,root,scale))return note;
 for(uint8_t d=1;d<12;d++){
  if(note>=d&&rupture_in_scale((uint8_t)(note-d),root,scale))return note-d;
  if((int)note+d<128&&rupture_in_scale((uint8_t)(note+d),root,scale))return note+d;
 }
 return note;
}
uint8_t rupture_arp_next(RuptureArp *a,const uint8_t *held,uint8_t count){
 if(!a||!held||!count)return 255;
 if(count>48)count=48;
 uint8_t sorted[48];for(uint8_t i=0;i<count;i++)sorted[i]=held[i];
 for(uint8_t i=1;i<count;i++){uint8_t x=sorted[i],j=i;while(j>0&&sorted[j-1]>x){sorted[j]=sorted[j-1];j--;}sorted[j]=x;}
 uint32_t full=(uint32_t)count*(a->octaves?a->octaves:1u);
 if(full>255u)full=255u;
 uint32_t idx=a->index%full;
 if(a->mode==RU_ARP_DOWN)idx=full-1u-idx;
 if(a->mode==RU_ARP_UPDOWN){uint32_t range=full<2u?1u:full*2u-2u;idx=a->index%range;if(idx>=full)idx=range-idx;}
 if(a->mode==RU_ARP_RANDOM){if(!a->rng)a->rng=0x9e3779b9u;a->rng^=a->rng<<13;a->rng^=a->rng>>17;a->rng^=a->rng<<5;idx=a->rng%full;}
 uint8_t note=a->mode==RU_ARP_ORDER?held[idx%count]:sorted[idx%count];
 unsigned oct=(a->mode==RU_ARP_ORDER)?a->index/count:idx/count;
 if((unsigned)note+12u*oct>127u)note=127u;else note=(uint8_t)(note+12u*oct);
 a->index++;return note;
}
