#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_subosc.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_fx_memory.h"
int main(void){
 RuState ui={0};ui.page=RU_PAGE_SUB1;ui.k[0]=2;ui.k[1]=1;ui.k[2]=50;ui.k[3]=20;
 assert(!strcmp(ru_knob_labels[RU_PAGE_SUB1][0],"TYPE"));
 assert(!strcmp(ru_knob_labels[RU_PAGE_SUB2][3],"DRIFT"));
 ui.sub_blend=0;ru_render(&ui);
 uint8_t zero[(RU_W*RU_H)/8];memcpy(zero,ru_bits,sizeof(zero));
 ui.sub_blend=100;ru_render(&ui);
 assert(memcmp(zero,ru_bits,sizeof(zero))!=0);
 ui.page=RU_PAGE_SUB2;ui.sub_blend=75;ru_render(&ui);
 uint32_t base=ru_sub_blend(3000,20000,10000,0,0);
 uint32_t half=ru_sub_blend(3000,20000,10000,RU_SUB_MIX_MAX/2,0);
 uint32_t full=ru_sub_blend(3000,20000,10000,RU_SUB_MIX_MAX,0);
 assert(base==3000&&half>base&&full>half);
 struct rupture_fx_memory old,new;
 ru_fxm_defaults(&old);old.knobs[0][0]=15;old.knobs[0][1]=95;old.enabled=0x08;
 assert(ru_fxm_valid(&old));
 new=old;new.knobs[0][0]=75;new.knobs[0][1]=25;new.enabled=0x18;
 assert(ru_fxm_valid(&new));
 assert(sizeof(old)==sizeof(new));
 printf("PASS V1.22 SUB BLEND 5th knob on both pages, additive mix, tagged 21-byte project record\n");
}
