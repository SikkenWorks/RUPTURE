/* Exact frame-state and octave timing regressions. */
#include <assert.h>
#include <stdint.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_dam_anim.h"
int main(void){
 RuDamAnim a={0};
 assert(!ru_dam_update(&a,0,0,0)&&a.frame==0);
 assert(ru_dam_update(&a,10,1,0)&&a.frame==1&&a.phase==RU_DAM_OPEN);
 assert(!ru_dam_update(&a,74,1,0)&&a.frame==1);
 assert(ru_dam_update(&a,75,1,0)&&a.frame==2);
 assert(ru_dam_update(&a,140,1,0)&&a.frame==3);
 assert(ru_dam_update(&a,205,1,0)&&a.frame==4);
 assert(ru_dam_update(&a,270,1,0)&&a.frame==5&&a.phase==RU_DAM_LEAK);
 assert(!ru_dam_update(&a,434,1,0)&&a.frame==5);
 assert(ru_dam_update(&a,435,1,0)&&a.frame==6);
 assert(ru_dam_update(&a,600,1,0)&&a.frame==7);
 assert(ru_dam_update(&a,765,1,0)&&a.frame==5); /* 3-frame loop */
 assert(ru_dam_update(&a,766,0,0)&&a.frame==8&&a.phase==RU_DAM_CLOSE);
 assert(ru_dam_update(&a,841,0,0)&&a.frame==9);
 assert(ru_dam_update(&a,916,0,0)&&a.frame==10);
 assert(ru_dam_update(&a,991,0,0)&&a.frame==11);
 assert(ru_dam_update(&a,1066,0,0)&&a.frame==0&&a.phase==RU_DAM_IDLE);
 assert(ru_dam_update(&a,1100,1,3)&&a.frame==1);
 assert(ru_dam_update(&a,1110,0,3)&&a.frame==10); /* short note closes hairline */
 assert(ru_dam_update(&a,1111,1,3)&&a.frame==1); /* retrigger */
 assert(ru_dam_leak_interval(-3)==270);
 assert(ru_dam_leak_interval(-2)==270);
 assert(ru_dam_leak_interval(-1)==220);
 assert(ru_dam_leak_interval(0)==165);
 assert(ru_dam_leak_interval(1)==115);
 assert(ru_dam_leak_interval(2)==75);
 assert(ru_dam_leak_interval(3)==55);
 assert(ru_dam_leak_interval(4)==45);
 assert(ru_dam_leak_interval(5)==35);
 assert(ru_dam_leak_interval(6)==35);
 return 0;
}
