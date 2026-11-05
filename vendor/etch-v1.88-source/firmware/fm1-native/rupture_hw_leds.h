/* SPDX-License-Identifier: GPL-3.0-only
 * RUPTURE V1.84.2: function LED ownership, only in the UI control loop.
 * Physical IDs: FX 2, SEL 3, ENV 4, LFO 5, EDIT 6, GLO 7,
 * HOME 8, SAVE 9, ARP 10, SEQ 11, PLAY 12, REC 13.
 * Unselected buttons retain the hardware 910 Hz low glow, not a slow blink.
 * This header performs no ISR, audio, storage or flash operation.
 */
#ifndef RUPTURE_HW_LEDS_H
#define RUPTURE_HW_LEDS_H
#include <stdint.h>
/* The UI is the owner of these page constants (rupture_ui.c). */
typedef struct { uint16_t solid; uint16_t glow; } RuFunctionLeds;
static inline RuFunctionLeds ru_function_leds(uint8_t page, uint8_t run,
    uint8_t recording, uint8_t countin, uint8_t countin_pulse,
    uint32_t physically_down, uint8_t protected_screen){
 const uint16_t function_bits=0x3ffcu; /* IDs 2..13 */
 uint16_t active=0u;
 if(protected_screen){
  /* A recoverable audio fault also drops the PC6 analogue gate.  The old
   * policy simultaneously switched ALL twelve button LEDs to full current,
   * adding a sharp power-rail load edge at exactly the amp-disable instant.
   * A protected page is signalled by the LCD; keep background LEDs at their
   * existing fast dim level, and preserve immediate physical press feedback.
   * This is also safe in Welcome, SAFE and updater states. */
  uint16_t down=(uint16_t)(physically_down&function_bits);
  RuFunctionLeds quiet={down,(uint16_t)(function_bits&~down)};
  return quiet;
 }
 switch(page){
  case 0: case 27: case 29: case 30: /* PLAY, SHAPE, SUB1, SUB2 */
  case 25: active|=(uint16_t)(1u<<8);break; /* MENU / HOME */
  case 2: active|=(uint16_t)(1u<<4);break; /* ENV */
  case 3: active|=(uint16_t)(1u<<5);break; /* LFO */
  case 4: case 5: case 6: case 7: case 8: case 31: case 32:
  case 34: case 35: case 36:
   active|=(uint16_t)(1u<<2);break; /* FX family */
  case 26: active|=(uint16_t)(1u<<6);break; /* EQ/EDIT */
  case 9: case 10: active|=(uint16_t)(1u<<11);break; /* SEQ */
  case 11: active|=(uint16_t)(1u<<10);break; /* ARP */
  case 15: case 16: case 17: case 37: case 38: case 39:
  case 40: case 41: case 42: case 43: case 44: case 45:
  case 46: case 47: case 48: case 50:
   active|=(uint16_t)(1u<<7);break; /* GLO and its children */
  default:break;
 }
 /* Transport and recording own their own status lights, on every page. */
 if(run)active|=(uint16_t)(1u<<12);
 if(recording)active|=(uint16_t)(1u<<13);
 if(countin){
  if(countin_pulse)active|=(uint16_t)(1u<<11);
  else active&=(uint16_t)~(1u<<11); /* pulsing SEQ countdown */
 }
 /* A pressed button is always immediately bright, even if it is not active. */
 active|=(uint16_t)(physically_down&function_bits);
 RuFunctionLeds result={active,(uint16_t)(function_bits&~active)};
 return result;
}

/* Boot-only function-button current-ramp.  The normal LED policy above is
 * untouched: this runs only in the existing UI thread, before the 10kHz
 * matrix scanner consumes the LED masks.  A first-run Welcome screen must
 * never command all twelve full-current LEDs simultaneously.  No new IRQ,
 * audio work, GPIO pin changes or 74HC595 timing changes are involved.
 *
 * The cold-start mask gradually introduces two low-current dim LEDs every
 * 40 ms, over about 240 ms. Once that is complete, only LEDs explicitly
 * selected by the original policy become fully lit. Physical button presses
 * remain immediately visible, including during the ramp.
 */
static inline RuFunctionLeds ru_function_led_boot_softstart(
    RuFunctionLeds requested, uint8_t welcome, uint32_t elapsed_ms,
    uint16_t pressed){
 const uint16_t function_bits=0x3ffcu;
 pressed=(uint16_t)(pressed&function_bits);
 if(welcome){
  /* V1.84 applied full current to every button in the Welcome wizard.
   * Give Welcome a low-current idle glow while preserving press feedback. */
  requested.solid=pressed;
  requested.glow=(uint16_t)(function_bits&~pressed);
 }
 if(elapsed_ms<260u){
  uint16_t permitted=0u;
  const uint32_t n=elapsed_ms/20u; /* one new low-current LED every 20 ms */
  for(uint32_t i=0u;i<n&&i<12u;i++)permitted|=(uint16_t)(1u<<(i+2u));
  /* During ramp there is no sudden full-current function-button group.
   * Do not delay an explicitly pressed button. */
  const uint16_t all_requested=(uint16_t)(requested.solid|requested.glow);
  RuFunctionLeds soft={
   (uint16_t)(requested.solid&pressed),
   (uint16_t)(all_requested&permitted&~pressed)
  };
  return soft;
 }
 requested.glow=(uint16_t)(requested.glow&~requested.solid);
 return requested;
}
#endif
