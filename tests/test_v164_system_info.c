#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c"
static uint32_t checksum(void){uint32_t h=2166136261u;for(unsigned i=0;i<sizeof(ru_bits);i++)h=(h^ru_bits[i])*16777619u;return h;}
int main(void){
 RuState v={0};v.page=RU_PAGE_SYS_INFO;v.system_battery_valid=1;
 v.system_usb=1;v.system_storage_ok=1;v.system_projects_used=3;v.battery=25;
 ru_render(&v);uint32_t low=checksum();
 v.battery=100;ru_render(&v);uint32_t full=checksum();assert(low!=full);
 v.system_battery_valid=0;ru_render(&v);assert(checksum()!=full);
 v.system_usb=0;ru_render(&v);uint32_t offline=checksum();
 v.system_usb=1;ru_render(&v);assert(offline!=checksum());
 puts("PASS V1.64 coarse battery bands, unknown ADC and real USB status UI");
}
