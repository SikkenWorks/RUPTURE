#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_subosc.h"
int main(void){
 for(int i=0;i<4;i++){
  const char *e[]={"TYPE","OCTAVE","DETUNE","DRIFT"};
  assert(!strcmp(ru_knob_labels[RU_PAGE_SUB1][i],e[i]));
  assert(!strcmp(ru_knob_labels[RU_PAGE_SUB2][i],e[i]));
 }
 assert(!strcmp(ru_knob_labels[RU_PAGE_NAME][0],"CURSOR"));
 assert(ru_sub_blend(12000,0,0,RU_SUB_MIX_MAX,RU_SUB_MIX_MAX)==12000);
 assert(ru_sub_blend(12000,12000,12000,RU_SUB_MIX_MAX,RU_SUB_MIX_MAX)>19000);
 assert(ru_sub_blend(12000,0,0,0,0)==12000);
 assert(ru_sub_blend(25000,25000,25000,RU_SUB_MIX_MAX,RU_SUB_MIX_MAX)<=32767);
 assert(ru_sub_blend(-25000,-25000,-25000,RU_SUB_MIX_MAX,RU_SUB_MIX_MAX)>=-32768);
 RuSubState a,b;ru_sub_init(&a,5300000,111);ru_sub_init(&b,5300000,222);
 long long energy_a=0,energy_b=0;long long sum_abs=0;
 for(int i=0;i<44100;i++){
  int v1=ru_sub_tick(&a,2,1,50,0,5300000,58,50,52,68);
  int v2=ru_sub_tick(&b,2,1,54,0,5300000,58,50,52,68);
  if(i>1000){energy_a+=(long long)v1*v1;energy_b+=(long long)v2*v2;sum_abs+=ru_sub_blend(1000,v1,v2,a.mix,b.mix)>1000?1:0;}
 }
 assert(energy_a>1000000000ll&&energy_b>1000000000ll);
 /* CLASSIC now carries odd harmonics that remain audible above its fundamental. */
 assert(ru_sub_classic_read(0x13579bdfu,5300000u)!=ru_sub_sine_read(0x13579bdfu));
 assert(sum_abs>1000);
 RuState u={0};u.page=RU_PAGE_SUB2;u.k[0]=2;u.k[1]=1;u.k[2]=54;u.k[3]=0;ru_render(&u);
 puts("PASS V1.10: SUB labels, independent classic audio, additive main unity, bounded peaks");return 0;
}
