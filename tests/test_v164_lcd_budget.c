#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_lcd_budget.h"
int main(void){
 assert(ru_lcd_animation_period(0,0)==50);
 assert(ru_lcd_animation_period(9,0)==100);
 assert(ru_lcd_animation_period(0,1)==100);
 assert(ru_lcd_animation_period(20,0)==180);
 assert(ru_lcd_animation_period(0,3)==180);
 assert(ru_lcd_idle_period(0,0)==350);
 assert(ru_lcd_idle_period(20,0)==750);
 assert(ru_lcd_should_paint(5,0,1,0,0,20,4)); /* input not delayed */
 assert(ru_lcd_should_paint(5,0,1,0,0,0,0));
 assert(!ru_lcd_should_paint(24,0,0,1,0,0,0));
 assert(ru_lcd_should_paint(25,0,0,1,0,0,0));
 assert(!ru_lcd_should_paint(50,0,0,1,0,20,3));
 assert(ru_lcd_should_paint(180,0,0,1,0,20,3));
 assert(!ru_lcd_should_paint(200,0,0,0,0,0,0)); /* idle savings */
 assert(ru_lcd_should_paint(350,0,0,0,0,0,0));
 assert(ru_lcd_should_paint(0,0xffffff00u,0,0,0,0,0)==0); /* wrap-safe */
 assert(ru_lcd_should_paint(95,0xfffffff0u,1,0,0,20,3));
 puts("PASS V1.64 audio-first LCD frame budget; urgent events, idle and stress");
}
