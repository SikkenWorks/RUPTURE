#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/audio_core.h"
static struct etch_native_audio s;
static struct etch_native_controls c;
static int16_t pcm[128*2];
static uint64_t h=1469598103934665603ull;
static double now(void){struct timespec ts;clock_gettime(CLOCK_MONOTONIC,&ts);return (double)ts.tv_sec+ts.tv_nsec/1e9;}
int main(void){
 etch_native_reset(&s);
 memset(&c,0,sizeof(c));
 etch_fx_defaults(&c.fx_settings);
 c.mono_mode=1; c.gain=30000;c.tempo=120;c.cutoff=16000;c.resonance=9000;c.filter_type=4;
 c.attack_step=0x00ffff00u;c.decay_step=0x00005500u;c.release_step=0x0000ff00u;c.sustain_level=23000;
 c.cxs_pressure=70;c.cxs_balance=65;c.cxs_memory=60;c.cxs_density=70;
 c.sub_type[0]=2;c.sub_type[1]=3;c.sub_blend[0]=60;c.sub_blend[1]=35;
 c.sub_octave[0]=1;c.sub_octave[1]=0;c.sub_detune[0]=48;c.sub_detune[1]=53;c.sub_drift[0]=50;c.sub_drift[1]=78;
 c.fx_settings.reverb_size=25000;c.fx_settings.reverb_decay=27000;
 etch_fx_set_reverb_type(&c.fx_settings,2);
 c.reverb=26000;c.chorus=21000;c.delay=14000;
 double t=now();
 for(int block=0;block<2200;block++){
  if((block%2)==0){etch_native_midi(&s,0x90,(uint8_t)(36+(block*3)%24),120);}
  else {etch_native_midi(&s,0x80,(uint8_t)(36+((block-1)*3)%24),0);}
  if((block%17)==0){ c.sub_drift[0]=(uint8_t)((block*7)%101);c.sub_drift[1]=(uint8_t)((block*11)%101); }
  etch_native_render_stereo(&s,&c,0,pcm,128);
  for(int j=0;j<256;j++){h^=(uint16_t)pcm[j];h*=1099511628211ull;}
 }
 /* Timed sequencer/ARP stress: test the exact sample on which a gated
  * performance note expires. Also exercise MIDI legato over seq playback. */
 static struct etch_native_pattern seq;
 for(int k=0;k<16;k++){
  seq.steps[k].count=(uint8_t)(k%4u!=3u);
  seq.steps[k].notes[0].note=(uint8_t)(36u+(k%7u));
  seq.steps[k].notes[0].velocity=110u;
  seq.steps[k].notes[0].length=(uint8_t)(1u+(k%3u));
 }
 etch_native_reset(&s);
 c.pattern=&seq;c.seq_run=1u;c.seq_length=16u;c.seq_rate=3u;
 c.seq_gate=67u;c.seq_swing=24u;c.tempo=240u;
 for(int block=0;block<1800;block++){
  if((block%31)==0)etch_native_midi(&s,0x90,(uint8_t)(40+block%17),115);
  if((block%31)==16)etch_native_midi(&s,0x80,(uint8_t)(40+(block-16)%17),0);
  if((block%350)==0 && block>0)c.seq_run=(uint8_t)(!c.seq_run);
  etch_native_render_stereo(&s,&c,0,pcm,128);
  for(int j=0;j<256;j++){h^=(uint16_t)pcm[j];h*=1099511628211ull;}
 }
 printf("PCM_HASH %016llx TIME_MS %.3f VOICES %u\n",(unsigned long long)h,(now()-t)*1000.,s.mono_count);
 return 0;
}
