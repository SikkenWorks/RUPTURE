#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_hw_leds.h"

int main(void){
 const uint32_t step=RU_LED_BOOT_WAVE_STEP_MS;
 assert(RU_LED_BOOT_WAVE_TOTAL_MS>1000u);
 assert(RU_LED_BOOT_WAVE_TOTAL_MS<1400u);
 for(uint8_t id=0u;id<41u;id++){
  uint32_t start=(uint32_t)id*step;
  if(start)assert(ru_led_boot_wave_level(start-1u,id,3u)==0u);
  assert(ru_led_boot_wave_level(start,id,3u)==1u);
  assert(ru_led_boot_wave_level(start+27u,id,3u)==2u);
  assert(ru_led_boot_wave_level(start+54u,id,3u)==3u);
  assert(ru_led_boot_wave_level(start+120u,id,3u)==2u);
  assert(ru_led_boot_wave_level(start+145u,id,3u)==1u);
  assert(ru_led_boot_wave_level(start+162u,id,3u)==3u);
  assert(ru_led_boot_wave_level(RU_LED_BOOT_WAVE_TOTAL_MS,id,0u)==0u);
 }
 /* The travelling front never lights all of the LEDs on one scan. */
 for(uint32_t time=0u;time<RU_LED_BOOT_WAVE_TOTAL_MS;time+=9u){
  int moving=0;
  for(uint8_t id=0u;id<41u;id++){
   uint8_t level=ru_led_boot_wave_level(time,id,0u);
   if(level)moving++;
  }
  assert(moving<=7);
 }
 /* Prior full-bright LED policy survives once the wave has completed. */
 for(uint8_t normal=0u;normal<=3u;normal++)
  for(uint8_t id=0u;id<41u;id++)
   assert(ru_led_boot_wave_level(RU_LED_BOOT_WAVE_TOTAL_MS,id,normal)==normal);
 puts("PASS V2.0.6 progressive 41-LED boot wave, tail, settled states");
 return 0;
}
