/* Original tone + transient test clip, processed with actual RUPTURE DSP. */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h"
static struct etch_native_fx fx;
static uint32_t seed=0x26c9u;
static uint32_t rand32(void){seed=1664525u*seed+1013904223u;return seed;}
static void u16(FILE *f,uint16_t v){fputc(v&255,f);fputc(v>>8,f);}
static void u32(FILE *f,uint32_t v){u16(f,(uint16_t)v);u16(f,(uint16_t)(v>>16));}
int main(int argc,char **argv){
 if(argc<4)return 2;
 int typ=atoi(argv[2]);int mix=atoi(argv[3]);if(typ<0||typ>2||mix<0||mix>100)return 2;
 int32_t count=44100*4;
 FILE *f=fopen(argv[1],"wb");if(!f)return 2;
 fwrite("RIFF",1,4,f);u32(f,(uint32_t)(36+count*4));
 fwrite("WAVEfmt ",1,8,f);u32(f,16);u16(f,1);u16(f,2);u32(f,44100);u32(f,44100*4);u16(f,4);u16(f,16);
 fwrite("data",1,4,f);u32(f,(uint32_t)(count*4));
 etch_fx_reset(&fx);struct etch_fx_settings s;etch_fx_defaults(&s);
 etch_fx_set_reverb_type(&s,(uint8_t)typ);s.reverb_size=28000;s.reverb_decay=24800;
 uint16_t wet=(uint16_t)((mix*32767u)/100u);
 for(int32_t i=0;i<count;i++){
  double time=i/44100.0;int32_t input=0;
  if(time<0.60){
   double env=(time<0.025?time/0.025:exp(-(time-0.025)*2.6));
   /* Soft original bass tone; no samples or copyrighted material. */
   input=(int32_t)(11000.0*env*(0.78*sin(time*2*3.141592653589793*82.41)+0.18*sin(time*2*3.141592653589793*164.82)+0.04*sin(time*2*3.141592653589793*247.23)));
  }
  if(time>1.20&&time<1.40){
   double env=exp(-(time-1.20)*33);
   int32_t noise=(int32_t)(rand32()>>16)-32768;
   input+=(int32_t)(env*9000.0*noise/32768.0);
  }
  int16_t l=0,r=0;
  etch_fx_process_stereo_masked(&fx,(int16_t)input,0,0,wet,&s,0,&l,&r);
  u16(f,(uint16_t)l);u16(f,(uint16_t)r);
 }
 fclose(f);return 0;
}
