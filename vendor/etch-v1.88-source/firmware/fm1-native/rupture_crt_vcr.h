/* RUPTURE V1.52: CRT optics and phosphor scanlines applied when converting
 * the existing 1-bit UI canvas into the existing RGB565 DMA stripe.
 * No removed glyph pixels, no new framebuffer, no heap, no audio IRQ calls.
 * All arithmetic is bounded integer, based on the exact 240x240 source. */
#ifndef RUPTURE_CRT_VCR_H
#define RUPTURE_CRT_VCR_H
#include <stdint.h>

#define RU_CRT_W 240u
#define RU_CRT_H 240u
#define RU_CRT_FULLWHITE 0xffffu
#define RU_CRT_SCANWHITE 0xdedbu /* slightly dimmer WHITE on every 4th display row */

/* The old implementation cut holes out of every fifth row of white glyphs.
 * This one instead samples from the original clean 1-bit canvas with a mild
 * 2D barrel lens. Result is still monochrome; the LCD RGB565 transfer gives
 * pale grey phosphor rows without breaking 1px stems or damaging text. */
/* The framebuffer is 1-bit; sample it bilinearly for the curved optical
 * output, preventing 1px glyph stems and horizontal rules from disappearing
 * at nearest-neighbour discontinuities. Faint edge pixels make CRT-like
 * phosphor fringes without introducing RGB color, animation or a second buffer. */
static inline uint32_t ru_crt_read(const uint8_t *bits,uint32_t x,uint32_t y){
 uint32_t i=y*RU_CRT_W+x;
 return (uint32_t)!!(bits[i>>3]&(uint8_t)(0x80u>>(i&7u)));
}
static inline uint16_t ru_crt_gray(uint32_t level){
 /* 0-255 neutral grayscale to RGB565, matching the FM1 LCD pixel format. */
 uint32_t r=(level*31u+127u)/255u;
 uint32_t g=(level*63u+127u)/255u;
 return (uint16_t)((r<<11)|(g<<5)|r);
}
static inline uint16_t ru_crt_pixel(const uint8_t *bits,uint32_t x,uint32_t y,uint8_t enabled){
 if(!enabled){
  return ru_crt_read(bits,x,y)?RU_CRT_FULLWHITE:0u;
 }
 int32_t dx=(int32_t)x-120,dy=(int32_t)y-120;
 uint32_t ax=(uint32_t)(dx<0?-dx:dx),ay=(uint32_t)(dy<0?-dy:dy);
 uint32_t r2=ax*ax+ay*ay;
 /* Q8 inverse barrel lens, bounded within ~7px at the corners. */
 int32_t sx=((int32_t)x<<8)+((dx*(int32_t)r2)>>11);
 int32_t sy=((int32_t)y<<8)+((dy*(int32_t)r2)>>11);
 if(sx<0||sy<0||sx>=((int32_t)(RU_CRT_W-1u)<<8)||sy>=((int32_t)(RU_CRT_H-1u)<<8))return 0u;
 uint32_t ix=(uint32_t)sx>>8,iy=(uint32_t)sy>>8;
 uint32_t fx=(uint32_t)sx&255u,fy=(uint32_t)sy&255u;
 uint32_t w0=(256u-fx)*ru_crt_read(bits,ix,iy)+fx*ru_crt_read(bits,ix+1u,iy);
 uint32_t w1=(256u-fx)*ru_crt_read(bits,ix,iy+1u)+fx*ru_crt_read(bits,ix+1u,iy+1u);
 uint32_t lum=((w0*(256u-fy)+w1*fy)*255u+32768u)>>16;
 /* No pixel deletion: dim existing light on every fourth row instead. */
 if((y&3u)==3u)lum=(lum*220u+127u)>>8;
 return ru_crt_gray(lum);
}
#endif
