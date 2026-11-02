/* Host-timing regressions for V3 20-frame user art; no audio engine involved. */
#include <assert.h>
#include <stdint.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_dam_anim.h"
int main(void){
 RuDamAnim a={0};
 assert(!ru_dam_update(&a,0,0,0)&&a.frame==0);
 assert(ru_dam_update(&a,10,1,0)&&a.frame==1&&a.phase==RU_DAM_OPEN);
 for(uint8_t f=2;f<=9;f++)assert(ru_dam_update(&a,10u+65u*(f-1u),1,0)&&a.frame==f);
 assert(ru_dam_update(&a,595,1,0)&&a.frame==10&&a.phase==RU_DAM_LEAK);
 assert(!ru_dam_update(&a,734,1,0));
 assert(ru_dam_update(&a,735,1,0)&&a.frame==11);
 assert(ru_dam_update(&a,875,1,0)&&a.frame==12);
 assert(ru_dam_update(&a,1015,1,0)&&a.frame==10);
 assert(ru_dam_update(&a,1016,0,0)&&a.frame==13&&a.phase==RU_DAM_CLOSE);
 for(uint8_t f=14;f<=19;f++)assert(ru_dam_update(&a,1016u+75u*(f-13u),0,0)&&a.frame==f);
 assert(ru_dam_update(&a,1541u,0,0)&&a.frame==0&&a.phase==RU_DAM_IDLE);
 assert(ru_dam_update(&a,1600,1,3)&&a.frame==1);
 assert(ru_dam_update(&a,1601,0,3)&&a.frame==18); /* short note never floods */
 assert(ru_dam_update(&a,1602,1,3)&&a.frame==1); /* retrigger */
 assert(ru_dam_leak_interval(-3)==250);
 assert(ru_dam_leak_interval(-2)==250);
 assert(ru_dam_leak_interval(-1)==200);
 assert(ru_dam_leak_interval(0)==140);
 assert(ru_dam_leak_interval(1)==110);
 assert(ru_dam_leak_interval(2)==85);
 assert(ru_dam_leak_interval(3)==65);
 assert(ru_dam_leak_interval(4)==50);
 assert(ru_dam_leak_interval(5)==40);
 assert(ru_dam_leak_interval(6)==40);
 return 0;
}
