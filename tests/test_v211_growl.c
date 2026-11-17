/* Reference-derived monophonic GROWL replaces only the MAIN SAW mode.
 * Host test verifies harmonic balance, safe sustained output, live control
 * sweeps, and ROM mip low/high-note safety. Not an FM1 DMA hardware test. */
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_growl_waves.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_growl_tone.h"

#define SR 44100u
#define FRAMES (SR*6u)

static double dft(int n){
 double re=0.0,im=0.0;
 for(int i=0;i<256;i++){
  double a=(2.*3.141592653589793*(double)(n*i))/256.;
  re+=(double)ru_growl_waves[6][i]*cos(a);
  im+=(double)ru_growl_waves[6][i]*sin(a);
 }
 return sqrt(re*re+im*im);
}

static void w16(FILE *f,unsigned n){fputc((int)(n&255u),f);fputc((int)((n>>8)&255u),f);}
static void w32(FILE *f,unsigned n){w16(f,n&65535u);w16(f,n>>16);}
static void wav_header(FILE *f,unsigned frames){
 fwrite("RIFF",1,4,f);w32(f,36u+frames*2u);
 fwrite("WAVEfmt ",1,8,f);w32(f,16u);
 w16(f,1u);w16(f,1u);w32(f,SR);w32(f,SR*2u);
 w16(f,2u);w16(f,16u);fwrite("data",1,4,f);w32(f,frames*2u);
}

int main(int argc,char **argv){
 double h1=dft(1),h2=dft(2),h3=dft(3),h4=dft(4);
 assert(h2>3.0*h1&&h3>4.0*h1&&h4>5.0*h1);
 assert(h4>h2&&h4>h3);
 /* At high notes the Nyquist-safe mip becomes a near-pure sine. */
 assert(ru_wave_band(0x72000000u)==0u);
 assert(ru_wave_band(0x00100000u)==6u);

 FILE *f=0;
 if(argc==3&&!strcmp(argv[1],"--preview")){
  f=fopen(argv[2],"wb");assert(f);wav_header(f,FRAMES);
 }
 RuGrowlState s={0,0};
 uint32_t phase=0;
 uint32_t increment=(uint32_t)(36.708095*4294967296./(double)SR);
 uint32_t h=2166136261u,active=0u,fullscale=0u,maxjump=0u;
 int32_t peak=0,prior=0;
 for(uint32_t n=0;n<FRAMES;n++){
  phase+=increment;
  /* Realtime BRIGHT sweeps mimic the expressivity in the video reference. */
  uint8_t bright=(uint8_t)(35u+(n/SR)%2u*35u+((n%SR)*15u)/SR);
  int32_t raw=ru_growl_read(phase,increment);
  int32_t output=rupture_growl_tone(&s,raw,bright,60u,52u,68u,62u,30u,24u,12u);
  assert(output>=-32767&&output<=32767);
  int32_t jump=abs(output-prior);if((uint32_t)jump>maxjump)maxjump=(uint32_t)jump;
  prior=output;
  int32_t a=abs(output);if(a>peak)peak=a;
  if(a>=32767)fullscale++;
  if(a>200)active++;
  h=(h^(uint16_t)output)*16777619u;
  if(f){
   /* Fade preview edges to avoid file-start/end clicks. */
   uint32_t fade=n<2205u?n:(n>FRAMES-2205u?FRAMES-n:2205u);
   int16_t y=(int16_t)((output*(int32_t)fade)/2205);
   w16(f,(uint16_t)y);
  }
 }
 if(f)fclose(f);
 assert(active>FRAMES/2u && peak>3000 && fullscale==0u);
 assert(maxjump<12000u);
 /* All eight controls at the 256 combinations of min/max, from bass to
  * higher notes: no arithmetic instability or output-domain overflow. */
 for(unsigned mask=0;mask<256u;mask++){
  RuGrowlState sweep={0,0};
  uint32_t phi=0u;
  uint32_t inc=4000000u+(mask%16u)*1280000u;
  uint8_t p[8];for(int i=0;i<8;i++)p[i]=(mask&(1u<<i))?100u:0u;
  for(int n=0;n<4096;n++){
   phi+=inc;
   int32_t out=rupture_growl_tone(&sweep,ru_growl_read(phi,inc),
       p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7]);
   assert(out>=-32767&&out<=32767);
  }
 }
 printf("PASS V2.0.11 growl: harmonics [%.2f %.2f %.2f %.2f], "
        "sustained peak=%d, fullscale=%u, maxjump=%u, hash=%u\n",
        h1/h4,h2/h4,h3/h4,1.0,peak,fullscale,maxjump,h);
 return 0;
}
