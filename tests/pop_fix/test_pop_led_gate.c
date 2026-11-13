/* Host-test RUPTURE pop-fix: de PC6-poort mag nooit openen zolang de
 * LED-bank nog niet gevestigd is (de LEDs worden nu ná de codec-opstart
 * gevestigd), en de nieuwe boot-insteltijd blijft binnen de bestaande poort. */
#include <assert.h>
#include <stdio.h>
#include "../../vendor/etch-v1.88-source/firmware/fm1-native/native_output_startup.h"
int main(void){
 /* LED-bank nog niet gezien -> nooit klaar, ongeacht verstreken tijd. */
 assert(!etch_output_led_quiet_ready(0u,0xFFFFFFF0u,0u));
 /* Gezien, maar < 400 ms stabiel -> niet klaar; daarna wel. */
 assert(!etch_output_led_quiet_ready(1u,ETCH_OUTPUT_LED_QUIET_TICKS-1u,0u));
 assert(etch_output_led_quiet_ready(1u,ETCH_OUTPUT_LED_QUIET_TICKS,0u));
 /* 300 ms stille codec-insteltijd (24 MHz) valt ruim binnen de bestaande
  * 1 s klok-settle: de PC6-tijdlijn wordt niet vervroegd. */
 assert((uint64_t)ETCH_OUTPUT_CODEC_PRE_LED_MS*24000u < ETCH_OUTPUT_CLOCK_SETTLE_TICKS);
 assert(ETCH_OUTPUT_CODEC_PRE_LED_MS>=100u && ETCH_OUTPUT_CODEC_PRE_LED_MS<=1000u);
 puts("PASS PC6-poort vereist gevestigde LED-bank; codec-insteltijd binnen klok-settle");
 return 0;
}
