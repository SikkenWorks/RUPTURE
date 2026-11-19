#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"

static struct etch_native_fx baseline,packed;

/* Every one of ten types gets a sparse late-slot test. Also test repeated
 * FX, dense racks, bypass ramps and mid-note changes at 128-frame edges. */
int main(void){
 struct etch_fx_settings c;
 etch_fx_defaults(&c);
 c.chorus_rate=13000;c.chorus_depth=22000;
 c.delay_time=16000;c.delay_feedback=18000;
 c.reverb_size=26000;c.reverb_decay=19000;
 c.folder_fold=23500;c.folder_mix=25000;
 c.comp_ratio=18000;c.comp_threshold=11000;
 c.wah_mix=24000;c.wah_sens=28000;
 rupture_tilt_controls(68,56,78,85,1);
 rupture_fuzz_controls(64,54,37,83,1);
 rupture_sandpaper_controls(73,53,66,81,1);
 uint32_t compared=0;
 for(uint8_t kind=0;kind<10;kind++)for(uint8_t layout=0;layout<12;layout++){
  struct rupture_fx_chain chain;
  ru_chain_clear(&chain);
  chain.independent=(uint8_t)(layout&1u);
  if(layout<8){
   assert(ru_chain_insert(&chain,(uint8_t)layout,kind));
  }else if(layout==8){
   assert(ru_chain_insert(&chain,0,kind));
   assert(ru_chain_insert(&chain,7,(uint8_t)((kind+3u)%10u)));
  }else if(layout==9){
   assert(ru_chain_insert(&chain,2,kind));
   assert(ru_chain_insert(&chain,5,(uint8_t)((kind+1u)%10u)));
   assert(ru_chain_insert(&chain,7,kind));
  }else if(layout==10){
   for(uint8_t i=0;i<8;i++)assert(ru_chain_insert(&chain,i,(uint8_t)((kind+i)%10u)));
  }else{
   /* Fully faded V1.89 COMP->TILT->FUZZ->SAND shortcut and empty gap. */
   for(uint8_t i=0;i<4;i++)assert(ru_chain_insert(&chain,i,(uint8_t)(RU_FX_COMP+i)));
   assert(ru_chain_insert(&chain,7,kind));
  }
  /* Independent FX exercise nonzero per-slot parameter banks. */
  for(uint8_t i=0;i<8;i++)if(chain.type[i]!=RU_FX_EMPTY){
   if(chain.type[i]==RU_FX_DISTORTION||chain.type[i]==RU_FX_WAH){
    chain.parameter[i][0]=21000;chain.parameter[i][1]=16000;
    chain.parameter[i][2]=24000;chain.parameter[i][3]=25000;
   }
  }
  etch_fx_reset(&baseline);
  etch_fx_reset(&packed);
  struct rupture_fx_dispatch dispatch={0};
  for(uint32_t frame=0;frame<8192;frame++){
   if(!(frame&127u)){
    if(frame==2048){
     /* Disabled inserts must remain in packed dispatch until fade ends. */
     chain.enabled&=(uint8_t)~(1u<<(layout<8?layout:7u));
    }
    if(frame==4096){
     chain.enabled=(uint8_t)(chain.enabled|0x80u);
     if(chain.type[7]==RU_FX_EMPTY)chain.enabled&=(uint8_t)~0x80u;
    }
    if(frame==6144 && layout<8)ru_chain_remove(&chain,(uint8_t)layout);
    dispatch=ru_chain_prepare_dispatch(&chain);
    assert(dispatch.span==ru_chain_span(&chain));
    assert(dispatch.count<=dispatch.span);
    for(uint8_t j=0;j<dispatch.count;j++){
     uint8_t slot=dispatch.slot[j];
     assert(slot<RU_FX_SLOTS && chain.type[slot]!=RU_FX_EMPTY);
     if(j)assert(slot>dispatch.slot[j-1u]);
    }
   }
   int16_t input=(int16_t)((frame%161u)<36u?26000:(frame&1u)?-14000:18000);
   int16_t l0,r0,l1,r1;
   rupture_fx_process_ordered_compact(&baseline,input,22000,21000,26000,18000,
     &c,&chain,0,&l0,&r0,ru_chain_span(&chain));
   rupture_fx_process_ordered_packed(&packed,input,22000,21000,26000,18000,
     &c,&chain,0,&l1,&r1,&dispatch);
   if(l0!=l1||r0!=r1){
    fprintf(stderr,"packed FX PCM difference: kind=%u layout=%u frame=%u baseline=%d/%d packed=%d/%d\n",
      kind,layout,frame,l0,r0,l1,r1);
    return 1;
   }
   compared++;
  }
 }
 struct rupture_fx_dispatch empty=ru_chain_prepare_dispatch(0);
 assert(!empty.count&&!empty.span);
 printf("PASS V2.0.22 packed FX: %u PCM stereo frames sample-identical\n",compared);
 return 0;
}
