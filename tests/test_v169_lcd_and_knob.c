/* RUPTURE V1.69 latency and original outline geometry. Pure host tests. */
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_lcd_budget.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_knob_ring_pixels.h"
int main(void){
 unsigned original=0;
 for(int y=-22;y<=22;y++)for(int x=-22;x<=22;x++){
  int r2=x*x+y*y;int old=(r2>=400&&r2<=484)||(r2>=272&&r2<=300&&((x+y)&1)==0);
  int now=0;for(unsigned z=0;z<RUPTURE_KNOB_RING_COUNT;z++)if(x==rupture_knob_ring_pixels[z][0]&&y==rupture_knob_ring_pixels[z][1])now++;
  assert(old==now);original+=old;
 }
 assert(original==332&&RUPTURE_KNOB_RING_COUNT==332);
 assert(ru_lcd_animation_period(0,0)==25);
 assert(ru_lcd_animation_period(20,0)==25); /* never throttle on max DSP alone */
 assert(ru_lcd_animation_period(0,1)==50);
 assert(ru_lcd_animation_period(20,3)==100); /* genuine DMA misses */
 assert(ru_lcd_should_paint(25,0,0,0,1,20,0));
 assert(ru_lcd_should_paint(25,0,0,1,0,20,0));
 assert(ru_lcd_should_paint(2,0,1,0,0,30,5)); /* immediate user input */
 assert(!ru_lcd_should_paint(25,0,0,1,0,20,3));
 assert(ru_lcd_should_paint(100,0,0,1,0,20,3));
 assert(!ru_lcd_should_paint(349,0,0,0,0,0,0));
 assert(ru_lcd_should_paint(350,0,0,0,0,0,0));
 printf("PASS V1.69 332 exact original knob pixels, healthy 25ms visual rate, true DMA deadline fallback\n");
 return 0;
}
