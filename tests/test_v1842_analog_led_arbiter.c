/* No hardware I/O. Regression: PC6 power edge cannot coincide with a new
 * full-current function-LED bank, even across the 32-bit TIMER4 wrap. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_hw_leds.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_output_startup.h"
#define L(i) (1u<<(i))
int main(void){
 RuFunctionLeds f=ru_function_leds(0,0,0,0,0,0,1);
 assert(f.solid==0 && f.glow==0x3ffcu);
 f=ru_function_leds(0,0,0,0,0,L(8),1);
 assert(f.solid==L(8) && !(f.glow&L(8)));
 for(uint32_t t=0;t<260;t+=20){
  f=ru_function_led_boot_softstart(f,1,t,L(8));
  assert((f.solid & ~L(8))==0);
 }
 assert(!etch_output_led_quiet_ready(0,100000000u,0));
 assert(!etch_output_led_quiet_ready(1,ETCH_OUTPUT_LED_QUIET_TICKS-1,0));
 assert(etch_output_led_quiet_ready(1,ETCH_OUTPUT_LED_QUIET_TICKS,0));
 assert(!etch_output_led_quiet_ready(1,50u,UINT32_MAX-100u));
 assert(etch_output_led_quiet_ready(1,ETCH_OUTPUT_LED_QUIET_TICKS-100u,UINT32_MAX-99u));
 puts("PASS V1.84.2: protected LED current, press feedback, PC6 stable-bank gate and wrap");
}
