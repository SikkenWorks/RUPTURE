/* RUPTURE V1.48 anti-alias guard: no tables/buffers or per-sample division.
 * Preserve default PCM and keep linear parameter sweeps bounded. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_bandwaves.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_saw_tone.h"
int main(void){
 assert(ru_saw_alias_strength(58,50,50,0,0)==0);
 assert(ru_saw_alias_strength(100,100,100,100,100)==100);
 uint32_t phase=0,inc=14732117u;
 int32_t low_old=0,low_new=0;
 uint64_t difference=0u;
 for(unsigned i=0;i<700000u;i++){
  phase+=inc;
  int32_t original=ru_bandwave_read(RU_WAVE_SAW,phase,inc);
  assert(ru_saw_guarded_read(phase,inc,0u)==original);
  int32_t a=rupture_saw_tone(&low_old,original,58,50,52,68,50,0,0,0);
  int32_t b=rupture_saw_tone(&low_new,ru_saw_guarded_read(phase,inc,0u),58,50,52,68,50,0,0,0);
  assert(a==b);
  uint8_t strength=(uint8_t)((i/1000u)%101u);
  int32_t shaped=ru_saw_guarded_read(phase,inc,strength);
  assert(shaped>=-32768 && shaped<=32767);
  if(strength>=90u){int32_t d=shaped-original;difference+=(uint32_t)(d<0?-d:d);}
 }
 assert(difference>100000u);
 for(uint8_t band=0;band<7;band++){
  uint32_t increment=(uint32_t)(0x7fffffffu>>(band+1u));
  for(unsigned i=0;i<1000u;i++){
   phase+=increment;
   int32_t y=ru_saw_guarded_read(phase,increment,(uint8_t)(i%101));
   assert(y>=-32768&&y<=32767);
  }
 }
 puts("PASS V1.48 SAW: neutral sample exact, 7 bands bounded, strong-drive guard responds");
 return 0;
}
