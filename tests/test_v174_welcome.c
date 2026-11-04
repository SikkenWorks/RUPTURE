/* RUPTURE V1.74: returning-user welcome is a legible, render-only phase. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c"
static unsigned pixels(void){unsigned k=0;for(unsigned i=0;i<sizeof ru_bits;i++)k+=__builtin_popcount((unsigned)ru_bits[i]);return k;}
int main(void){
 RuState v={0};v.page=RU_PAGE_WELCOME;v.welcome_phase=RU_WELCOME_RETURN;
 ru_render(&v);unsigned returning=pixels();assert(returning>200u);
 v.welcome_phase=RU_WELCOME_INTRO;ru_render(&v);unsigned first=pixels();assert(first>200u&&first!=returning);
 assert(RU_WELCOME_RETURN!=RU_WELCOME_INTRO);
 assert(RU_WELCOME_RETURN!=RU_WELCOME_FORMAT);
 puts("PASS V1.74: returning-user 1-bit welcome preview and independent first-run wizard phases");
 return 0;
}
