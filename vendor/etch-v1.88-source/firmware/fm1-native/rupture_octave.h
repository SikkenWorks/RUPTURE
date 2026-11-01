/* SPDX-License-Identifier: GPL-3.0-only
 * RUPTURE octave reference: the ETCH physical -2 bass register is RUPTURE 0.
 * Keep the musical pitch as an ABSOLUTE value in native_controls.octave.
 * Translate ONLY indicator values / UI labels; do not transpose MIDI twice.
 */
#ifndef RUPTURE_OCTAVE_H
#define RUPTURE_OCTAVE_H
#include <stdint.h>
#define RUPTURE_BASS_OCTAVE (-2)
#define RUPTURE_ABSOLUTE_OCT_MIN (-4)
#define RUPTURE_ABSOLUTE_OCT_MAX (3)
/* Visible positions therefore range -2 .. +5, with the familiar low note at 0. */
static inline int8_t rupture_octave_display(int8_t absolute_octave){
 return (int8_t)(absolute_octave-RUPTURE_BASS_OCTAVE);
}
static inline int8_t rupture_octave_on_press(int8_t absolute_octave,uint32_t pressed){
 const uint32_t oct=pressed&3u;
 if(oct==1u && absolute_octave>RUPTURE_ABSOLUTE_OCT_MIN)--absolute_octave;
 else if(oct==2u && absolute_octave<RUPTURE_ABSOLUTE_OCT_MAX)++absolute_octave;
 return absolute_octave;
}
/* LEDs signal the distance from the RUPTURE baseline, not ETCH's baseline.
   Preserve the immediate held-button feedback even if still at zero. */
static inline uint8_t rupture_octave_led(uint8_t button,int8_t absolute_octave,uint32_t pressed){
 const int8_t shown=rupture_octave_display(absolute_octave);
 return (uint8_t)(button==0u?(shown<0 || (pressed&1u)):(shown>0 || (pressed&2u)));
}
#endif
