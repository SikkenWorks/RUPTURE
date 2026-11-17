/* RUPTURE V2.0.12: strong sonic variation, valid headroom, long-note stability.
 * No claim is made about FM1 hardware/DMA without a physical FM1 test. */
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_growl_waves.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_growl_tone.h"

#define SR 44100u
#define SCENE_SAMPLES (SR*2u)

struct Scene {const char *name;uint8_t bright,edge,body,level,shape_edge,weight,bite,punch;};
static const struct Scene scenes[]={
 {"DARK / CLEAN",0,0,55,68,25,55,0,20},
 {"BRIGHT / CLEAN",100,0,55,68,25,55,0,20},
 {"BRIGHT / EDGE 100",100,100,55,68,25,55,0,20},
 {"BRIGHT / BODY 100",100,100,100,68,25,55,0,20},
 {"BITE / FORMANT 100",100,100,100,68,100,55,100,20},
 {"FULL GROWL",100,100,100,68,100,100,100,100},
};

static void write16(FILE *f,unsigned n){fputc((int)n&255,f);fputc((int)(n>>8)&255,f);}
static void write32(FILE *f,uint32_t n){write16(f,n&65535u);write16(f,n>>16);}
static void wavhead(FILE *f,uint32_t frames){
 fwrite("RIFF",1,4,f);write32(f,36u+frames*2u);
 fwrite("WAVEfmt ",1,8,f);write32(f,16u);write16(f,1u);write16(f,1u);
 write32(f,SR);write32(f,2u*SR);write16(f,2u);write16(f,16u);
 fwrite("data",1,4,f);write32(f,2u*frames);
}
int main(int argc,char **argv){
 FILE *wav=0;
 if(argc==3&&!strcmp(argv[1],"--preview")){
  wav=fopen(argv[2],"wb");assert(wav);
  wavhead(wav,SCENE_SAMPLES*(uint32_t)(sizeof(scenes)/sizeof(scenes[0])));
 }
 double brightness[2]={0},slope[6]={0},rms[6]={0};
 uint32_t maxpeak=0,fullscale=0,maxjump=0,where=0,which=0;
 const uint32_t inc=(uint32_t)(36.708095*4294967296.0/(double)SR);
 for(unsigned scene=0;scene<sizeof(scenes)/sizeof(scenes[0]);scene++){
  const struct Scene *c=&scenes[scene];
  RuGrowlParams p=ru_growl_make_params(c->bright,c->edge,c->body,c->level,
                            c->shape_edge,c->weight,c->bite,c->punch);
  RuGrowlState state={0};
  uint32_t phase=0;
  int32_t prior=0;
  double energy=0,diff=0;
  for(uint32_t n=0;n<SCENE_SAMPLES;n++){
   phase+=inc;
   int32_t raw=ru_growl_read(phase,inc);
   int32_t out=ru_growl_process(&state,raw,&p);
   uint32_t peak=(uint32_t)abs(out);
   if(peak>maxpeak)maxpeak=peak;
   if(peak>=32767u)fullscale++;
   uint32_t jump=(uint32_t)abs(out-prior);
   /* The FM1 applies its pre-existing per-note attack fade on note-on.
    * Host oscillator-only renders do not have that envelope. */
   if(n>=128u && jump>maxjump){maxjump=jump;where=n;which=scene;}
   if(n>SR){energy+=(double)out*out;diff+=(double)(out-prior)*(out-prior);}
   prior=out;
   if(wav){
    /* Discrete sections use 80ms fades to avoid a misleading edit pop. */
    uint32_t fade=SR/12u;
    if(n<fade)out=(out*(int32_t)n)/(int32_t)fade;
    else if(n>SCENE_SAMPLES-fade)out=(out*(int32_t)(SCENE_SAMPLES-n))/(int32_t)fade;
    write16(wav,(uint16_t)(int16_t)out);
   }
  }
  rms[scene]=sqrt(energy/(double)(SCENE_SAMPLES-SR));
  slope[scene]=sqrt(diff/energy);
  if(scene<2)brightness[scene]=slope[scene];
  printf("%s | rms %.0f | normalized edge %.3f\n",c->name,rms[scene],slope[scene]);
 }
 if(wav)fclose(wav);
 printf("Diagnostics: peak %u, fullscale %u, maxjump %u at scene %u frame %u\n",
        maxpeak,fullscale,maxjump,which,where);
 assert(!fullscale && maxpeak<=31000u && maxjump<20000u);
 assert(brightness[1]>brightness[0]*1.12);
 /* Upper bands and asymmetric nonlinear drive must visibly change PCM,
  * not merely rename SAW or move a nearly inaudible gain coefficient. */
 assert(fabs(slope[2]-slope[1])>0.01);
 assert(fabs(rms[3]-rms[2])>10.0 || fabs(slope[3]-slope[2])>0.005);
 assert(fabs(slope[4]-slope[3])>0.01);
 /* Value=0 on the LEVEL knob is a real zero, even at all other maxima. */
 RuGrowlParams muted=ru_growl_make_params(100,100,100,0,100,100,100,100);
 RuGrowlState silence={0};uint32_t phase=0;
 for(uint32_t n=0;n<SR;n++){
  phase+=inc;
  assert(ru_growl_process(&silence,ru_growl_read(phase,inc),&muted)==0);
 }
 /* Individually changing ANY control from 50 to 100 must alter the
  * actual held-note waveform by at least 5% RMS of the original signal.
  * This guards against controls that exist in UI but sound inert. */
 uint8_t base[8]={50u,50u,50u,68u,50u,50u,50u,50u};
 int16_t baseline[2048];
 RuGrowlParams bp=ru_growl_make_params(base[0],base[1],base[2],base[3],
                          base[4],base[5],base[6],base[7]);
 RuGrowlState bs={0};uint32_t bphase=0;
 double reference_energy=0;
 for(uint32_t n=0;n<10240u;n++){
  bphase+=inc;
  int32_t v=ru_growl_process(&bs,ru_growl_read(bphase,inc),&bp);
  if(n>=8192u){
   baseline[n-8192u]=(int16_t)v;
   reference_energy+=(double)v*v;
  }
 }
 for(unsigned changed=0;changed<8u;changed++){
  uint8_t controls[8];memcpy(controls,base,sizeof(controls));
  controls[changed]=100u;
  RuGrowlParams cp=ru_growl_make_params(controls[0],controls[1],controls[2],
                 controls[3],controls[4],controls[5],controls[6],controls[7]);
  RuGrowlState cs={0};uint32_t cphase=0;
  double delta_energy=0;
  for(uint32_t n=0;n<10240u;n++){
   cphase+=inc;
   int32_t v=ru_growl_process(&cs,ru_growl_read(cphase,inc),&cp);
   if(n>=8192u){
    double d=(double)v-baseline[n-8192u];delta_energy+=d*d;
   }
  }
  double ratio=sqrt(delta_energy/reference_energy);
  printf("Control %u @100 versus @50: PCM variation %.1f%%\n",
         changed,(double)ratio*100.0);
  assert(ratio>0.05);
 }
 /* 0/100 extremes at six bass and higher pitches with long held notes. */
 for(unsigned pitch=0;pitch<6;pitch++)for(unsigned mask=0;mask<256;mask++){
  uint8_t pval[8];for(int i=0;i<8;i++)pval[i]=(mask&(1u<<i))?100u:0u;
  RuGrowlParams control=ru_growl_make_params(pval[0],pval[1],pval[2],pval[3],
                               pval[4],pval[5],pval[6],pval[7]);
  RuGrowlState state={0};uint32_t phi=0;
  uint32_t step=1200000u+pitch*5100000u;
  for(int k=0;k<4096;k++){
   phi+=step;
   int32_t out=ru_growl_process(&state,ru_growl_read(phi,step),&control);
   assert(out>=-31000&&out<=31000);
  }
 }
 printf("PASS V2.0.12: expressive controls, %u full-scale, peak %u, max jump %u, 1536 sustained parameter cases\n",
  fullscale,maxpeak,maxjump);
 return 0;
}
