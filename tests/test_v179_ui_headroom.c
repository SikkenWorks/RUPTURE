#include <assert.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_lcd_budget.h"
int main(void){
 /* Healthy UI still responds to every 25-ms input service. */
 assert(!ru_lcd_knob_paint_due(24,0,0,0));
 assert(ru_lcd_knob_paint_due(25,0,0,0));
 /* Recent AUDIO wall-time pressure, not a historical DSP MAX reading. */
 assert(!ru_lcd_knob_paint_due(49,0,15,0));
 assert(ru_lcd_knob_paint_due(50,0,15,0));
 assert(!ru_lcd_knob_paint_due(74,0,27,0));
 assert(ru_lcd_knob_paint_due(75,0,27,0));
 assert(!ru_lcd_knob_paint_due(99,0,0,3));
 assert(ru_lcd_knob_paint_due(100,0,0,3));
 assert(ru_lcd_knob_paint_due(80,0xfffffffeu,27,1));
 /* Also protect DMA against redundant idle waveform/sprite transfers.
  * Animation state continues to advance even when one LCD frame is skipped. */
 assert(ru_lcd_animation_period(30,0)==50u);
 assert(!ru_lcd_should_paint(25,0,0,1,1,30,0));
 assert(ru_lcd_should_paint(50,0,0,1,1,30,0));
 /* Every press/fault/page switch still forces an immediate display. */
 assert(ru_lcd_should_paint(1,0,1,0,0,32,8));
 puts("PASS V1.79: coalesce LCD knob redraws under audio wall-time pressure; urgent UI preserved");
}
