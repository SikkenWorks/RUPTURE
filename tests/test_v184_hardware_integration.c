/* Host-only checks for physical RUPTURE function LEDs and safe detent feel. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_hw_leds.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_encoder_feel.h"
#define L(n) (1u << (n))
static void check(uint8_t page,uint8_t button){
 RuFunctionLeds l=ru_function_leds(page,0,0,0,0,0,0);
 assert(l.solid&L(button));
 assert((l.solid&l.glow)==0u);
 assert((l.solid|l.glow)==0x3ffcu);
}
int main(void){
 check(0,8);check(2,4);check(3,5);check(4,2);check(5,2);
 check(26,6);check(9,11);check(10,11);check(11,10);
 check(15,7);check(17,7);check(29,8);check(36,2);
 RuFunctionLeds l=ru_function_leds(9,1,1,0,0,0,0);
 assert((l.solid&(L(11)|L(12)|L(13)))==(L(11)|L(12)|L(13)));
 l=ru_function_leds(9,0,1,1,0,0,0);
 assert((l.solid&L(11))==0u && (l.glow&L(11))!=0u && (l.solid&L(13))!=0u);
 l=ru_function_leds(9,0,1,1,1,0,0);
 assert((l.solid&L(11))!=0u);
 l=ru_function_leds(2,0,0,0,0,L(10),0);
 assert((l.solid&L(10))!=0u && (l.solid&L(4))!=0u);
 l=ru_function_leds(49,0,0,0,0,0,1);
 assert(l.solid==0u && l.glow==0x3ffcu); /* no all-on spike at protected/fault status */
 RuEncoderFeel e={0};
 assert(ru_encoder_feel(&e,1000,1,1)==1);
 assert(ru_encoder_feel(&e,1030,1,1)==1);
 assert(ru_encoder_feel(&e,1060,1,1)==2);
 e=(RuEncoderFeel){0}; /* page switch resets boost history */
 assert(ru_encoder_feel(&e,1070,1,1)==1);
 assert(ru_encoder_feel(&e,1080,1,0)==1); /* discrete never boosted */
 puts("PASS V1.84 function ownership, pressed buttons, countdown, safety and page-change encoder reset");
 return 0;
}
