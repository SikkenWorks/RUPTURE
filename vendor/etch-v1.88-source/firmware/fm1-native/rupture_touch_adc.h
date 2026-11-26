/* SPDX-License-Identifier: GPL-3.0-only
 * V2.0.47 optional contact-resistance sensing, inspired by Velo's measured
 * PA7/PA8 drive technique (C. Vestal, GPL-3.0-only). Intentionally separate
 * from the audited Felucca key-scan HAL: digital debounce and all LED/key
 * scan timing remain owned by fm1_input_tick(), even if this ADC fails.
 * The single SARADC belongs to the timer tick only while enabled.
 */
#ifndef RUPTURE_TOUCH_ADC_H
#define RUPTURE_TOUCH_ADC_H
#include <stdint.h>
#include "hal/fm1_adc.h"

static volatile uint8_t fm1_touch_hw_on, fm1_touch_hw_fault;
static volatile uint16_t fm1_touch_raw[20],fm1_touch_gen[20];
static volatile uint16_t fm1_touch_master=512u,fm1_touch_battery=560u;
static volatile uint8_t fm1_touch_master_valid,fm1_touch_battery_valid;
static uint8_t fm1_touch_pending,fm1_touch_pin,fm1_touch_key;
static uint8_t fm1_touch_failures;

/* Called at the beginning of TIMER5, BEFORE the ordinary digital key scan.
 * The row is restored to input even on timeout or when the option was
 * turned OFF while a conversion was pending. This is a note-off invariant. */
static inline void ru_touch_adc_before_scan(void){
 if(!fm1_touch_pending)return;
 const uint8_t ch=fm1_touch_pin;
 fm1_touch_pending=0u;
 if(ch<=1u){
  uint32_t m=1u<<(ch?8u:7u);
  FM1_PR(FM1_PA,FM1_DIR)|=m;
  FM1_PR(FM1_PA,FM1_OUT)&=~m;
  FM1_PR(FM1_PA,FM1_HD0)&=~m;
  FM1_PR(FM1_PA,FM1_HD)&=~m;
 }
 if(FM1_ADC_CON&0x80u){
  uint16_t v=(uint16_t)(FM1_ADC_RES&0x3ffu);
  FM1_ADC_CON=0x40u;FM1_ADC_CON=0u;
  fm1_touch_failures=0u;
  if(ch<=1u&&fm1_touch_key<20u){
   fm1_touch_raw[fm1_touch_key]=v;
   fm1_touch_gen[fm1_touch_key]++;
  }else if(ch==4u){fm1_touch_master=v;fm1_touch_master_valid=1u;
  }else if(ch==3u){fm1_touch_battery=v;fm1_touch_battery_valid=1u;}
 }else{
  FM1_ADC_CON=0x40u;FM1_ADC_CON=0u;
  if(fm1_touch_failures<255u)fm1_touch_failures++;
  if(fm1_touch_failures>=8u){
   /* Stop all optional sampling, never stop notes, USB or the codec. */
   fm1_touch_hw_on=0u;fm1_touch_hw_fault=1u;
  }
 }
}

/* Called AFTER fm1_input_tick has finished the digital scan and latched the
 * next column. All ADC conversions finish in hardware between TIMER5 ticks;
 * there is NO busy wait in the timer interrupt. Nested audio work wins.
 * Only a held note from the PA7/PA8 rows gets a resistance measurement.
 * The top seven notes have no analog row and retain fixed expression. */
static inline void ru_touch_adc_after_scan(uint8_t audio_nested){
 if(!fm1_touch_hw_on||fm1_touch_pending||audio_nested)return;
 uint8_t col=fm1__tick_col;
 uint32_t frames=fm1_in.frames;
 uint8_t ch=255u,key=255u;
 if(col==0u && (frames&511u)==0u)ch=3u; /* battery ~0.56 s */
 else if(col==0u && (frames&7u)==0u)ch=4u; /* master ~9 ms */
 else{
  uint8_t row=(uint8_t)((frames&1u)?4u:3u);
  int8_t id=FM1_KEYMAP[row][col];
  if(id>=14&&id<34&&(fm1_in.notes&(1u<<(id-14)))){
   ch=(uint8_t)(row==3u?0u:1u);
   key=(uint8_t)(id-14);
  }
 }
 if(ch==255u)return;
 if(ch<=1u){
  /* Suppress the shared LED line only on this sampled, held key's column.
   * The original scan restores its normal full/animation/dim level on the
   * next tick. Other columns and the complete boot sweep are unchanged. */
  uint8_t rowbit=(uint8_t)(ch?4u:3u);
  fm1__led_lines(FM1__LIT(col)&~(1u<<rowbit));
  /* Velo's measured DRIVE 2 separates light/firm carbon contacts much
   * better than the old weakest drive (0). Reset both strength bits
   * immediately before the next digital scan, including ADC timeouts. */
  uint32_t m=1u<<(ch?8u:7u);
  FM1_PR(FM1_PA,FM1_HD0)&=~m;
  FM1_PR(FM1_PA,FM1_HD)|=m;
  FM1_PR(FM1_PA,FM1_OUT)|=m;
  FM1_PR(FM1_PA,FM1_DIR)&=~m;
 }
 fm1_touch_pin=ch;fm1_touch_key=key;
 FM1_ADC_CON=0u;
 if(FM1_WLA_CON0&(1u<<14))FM1_WLA_CON0&=~(1u<<14);
 /* ADC divider 7, delay 2: old 0x204F actually selected divider 15,
  * which could miss the 100us key-scan deadline and disable sensing. */
 FM1_ADC_CON=0x2047u|((uint32_t)ch<<8);
 FM1_ADC_CON|=0x10u;
 FM1_ADC_CON|=0x40u;
 fm1_touch_pending=1u;
}

static inline void ru_touch_adc_enable(uint8_t on){
 fm1_touch_hw_fault=0u;fm1_touch_failures=0u;
 fm1_touch_hw_on=(uint8_t)!!on;
}

/* Foreground-only snapshot under a brief TIMER5 lock. NEVER used in ISR
 * or the audio sample loop. Generation distinguishes real fresh readings. */
static inline void ru_touch_adc_snapshot(uint16_t samples[20],uint16_t generations[20]){
 fm1_irq_off();
 for(uint8_t i=0u;i<20u;i++){
  samples[i]=fm1_touch_raw[i];generations[i]=fm1_touch_gen[i];
 }
 fm1_irq_enable_all();
}
#endif
