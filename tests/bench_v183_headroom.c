#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/audio_core.h"

static struct etch_native_audio audio;
static struct etch_native_controls c;
static int16_t output[128*2];
static uint64_t hash=1469598103934665603ull;
static uint32_t peak;
static void acc(void) {
 for(unsigned j=0;j<128*2;j++){
  int32_t v=output[j]; if((uint32_t)(v<0?-v:v)>peak)peak=(uint32_t)(v<0?-v:v);
  hash^=(uint16_t)v;hash*=1099511628211ull;
 }
}
int main(void){
 etch_native_reset(&audio);
 memset(&c,0,sizeof(c));etch_fx_defaults(&c.fx_settings);
 c.mono_mode=1;c.gain=26000;c.tempo=158;c.cutoff=28000;
 c.resonance=11000;c.filter_type=4;c.attack_step=0x00ffff00u;
 c.decay_step=0x0000ffffu;c.release_step=0x00005200u;c.sustain_level=25000;
 c.cxs_pressure=76;c.cxs_balance=48;c.cxs_edge=69;
 c.sub_type[0]=2;c.sub_type[1]=3;c.sub_blend[0]=65;c.sub_blend[1]=50;
 c.sub_drift[0]=40;c.sub_drift[1]=59;c.sub_detune[0]=38;c.sub_detune[1]=51;
 c.reverb=24000;c.chorus=18000;c.delay=13000;
 c.fx_settings.reverb_decay=28000;c.fx_settings.reverb_size=21000;
 c.fx_settings.phaser_mix=18000;c.fx_settings.tape_mix=16000;
 c.fx_settings.vibrato_mix=14000;c.fx_settings.decim_mix=12500;
 struct etch_native_pattern pat={0};
 for(unsigned j=0;j<16;j++){
  pat.steps[j].count=(uint8_t)((j%6u)!=0u);
  pat.steps[j].notes[0].note=(uint8_t)(36u+(j*5u)%14u);
  pat.steps[j].notes[0].velocity=(uint8_t)(80u+(j*3u)%48u);
  pat.steps[j].notes[0].length=(uint8_t)(1u+j%3u);
 }
 for(unsigned b=0;b<450;b++){
  if(b%3==0)etch_native_midi(&audio,0x90,(uint8_t)(34u+(b%37u)),(uint8_t)(100u+(b%25u)));
  if(b%3==1)etch_native_midi(&audio,0x80,(uint8_t)(34u+((b-1u)%37u)),0);
  if(b%200==0)etch_native_midi(&audio,0xb0,64,127);
  if(b%200==130)etch_native_midi(&audio,0xb0,64,0);
  if(b%11==0){c.sub_drift[0]=(uint8_t)((b*11u)%101u);c.sub_drift[1]=(uint8_t)((b*23u)%101u);}
  if(b%59==0)etch_fx_set_reverb_type(&c.fx_settings,(uint8_t)((b/59u)%3u));
  if(b%29==0)c.gain=(uint16_t)(10000u+((b*113u)%22768u));
  if(b%17==0)c.drive=(uint16_t)((b*73u)%27000u);
  if(b%401==0)c.preview_wave=1;
  if(b%401==14)c.preview_wave=0;
  if(b%333==0){c.pattern=&pat;c.seq_run=1;c.seq_length=16;c.seq_rate=3;c.seq_gate=70;c.seq_swing=24;}
  if(b%333==200)c.seq_run=0;
  etch_native_render_stereo(&audio,&c,0u,output,128);acc();
 }
 printf("AUDIO_HASH %016llx PEAK %u FRAMES %u\n",(unsigned long long)hash,peak,450u*128u);
 return 0;
}
