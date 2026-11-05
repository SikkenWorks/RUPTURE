/* Host regression for boot-only function LED slew; all non-LED code stays unmodified. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_hw_leds.h"
#define LED(n) (1u << (n))
static unsigned count(unsigned n){unsigned c=0;while(n){c+=(n&1u);n>>=1;}return c;}
int main(void){
 const RuFunctionLeds normal=ru_function_leds(0,0,0,0,0,0,0);
 const RuFunctionLeds protected=ru_function_leds(0,0,0,0,0,0,1);
 assert(normal.solid==LED(8));
 assert(protected.solid==0u && protected.glow==0x3ffcu); /* protected-state current spike fixed */
 RuFunctionLeds p=ru_function_led_boot_softstart(protected,1,0,0);
 assert(p.solid==0 && p.glow==0);
 for(unsigned t=0;t<260;t+=20){
  RuFunctionLeds v=ru_function_led_boot_softstart(protected,1,t,0);
  assert(v.solid==0); /* no simultaneous high-current bank at welcome */
  assert(count(v.glow)<=t/20);
 }
 p=ru_function_led_boot_softstart(protected,1,300,0);
 assert(p.solid==0 && p.glow==0x3ffcu);
 p=ru_function_led_boot_softstart(normal,0,260,0);
 assert(p.solid==normal.solid && p.glow==normal.glow); /* regular logic returns unchanged */
 p=ru_function_led_boot_softstart(normal,0,20,0);
 assert(p.solid==0 && count(p.glow)==1);
 p=ru_function_led_boot_softstart(normal,0,0,LED(10));
 assert((p.solid&LED(10))==0u); /* this request did not include a press */
 RuFunctionLeds pressed=ru_function_leds(0,0,0,0,0,LED(10),0);
 p=ru_function_led_boot_softstart(pressed,0,0,LED(10));
 assert(p.solid&LED(10)); /* a physical button remains instant */
 for(unsigned t=260;t<5000;t+=37){
  p=ru_function_led_boot_softstart(normal,0,t,0);
  assert(p.solid==normal.solid && p.glow==normal.glow);
 }
 puts("PASS: boot function LEDs ramp from dark to dim, Welcome has no full-current group; normal LED behavior unchanged");
 return 0;
}
