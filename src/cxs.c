/* SPDX-License-Identifier: GPL-3.0-only */
#include "cxs.h"
#include <math.h>
#include <string.h>
#define CXS_PI 3.14159265358979323846f
static float clampf(float x,float lo,float hi){return x<lo?lo:x>hi?hi:x;}
static float midi_hz(int n){return 440.0f*powf(2.0f,((float)n-69.0f)/12.0f);}
static float blep(float p,float dt){
 if(p<dt){float x=p/dt;return x+x-x*x-1.0f;}
 if(p>1.0f-dt){float x=(p-1.0f)/dt;return x*x+x+x+1.0f;}
 return 0.0f;
}
static uint32_t rand32(CXSEngine *e){e->seed^=e->seed<<13;e->seed^=e->seed>>17;e->seed^=e->seed<<5;return e->seed;}
/* Pairwise transfer conserves exactly the period sum (apart from float rounding). */
static void exchange(CXSEngine *e){
 float pressure=clampf(e->knob.pressure*.01f,0,1);
 float balance=clampf((e->knob.balance-50)*.02f,-1,1);
 float memory=clampf(e->knob.memory*.01f,0,1);
 float retention=.05f+.93f*memory;
 for(int i=0;i<CXS_SEGMENTS;i++){
  int j=(i+1)&7;
  float noise=((float)(rand32(e)&65535u)/32767.5f)-1.0f;
  float bias=balance*(i&1?-.6f:.6f);
  float restoring=(e->d[j]-e->d[i])*1.5f;
  float requested=(noise*.18f+bias*.32f+restoring)*.014f*pressure;
  e->flux[i]=e->flux[i]*retention+requested*(1-retention);
  float transfer=clampf(e->flux[i],-(e->d[j]-.020f),e->d[i]-.020f);
  e->d[i]-=transfer;e->d[j]+=transfer;
 }
 float x=0;for(int i=0;i<CXS_SEGMENTS;i++){e->edge[i]=x;x+=e->d[i];}
 /* Dense mode creates more pronounced alternate levels, not extra voices. */
 float density=clampf(e->knob.density*.01f,0,1);
 for(int i=0;i<CXS_SEGMENTS;i++){
  float shape=(i&1)?-.86f:1.0f;
  float partial=(1.0f-density)*((i&3)<2?1.0f:-1.0f)+density*shape;
  e->levels[i]=partial*.78f;
 }
 e->cycle++;
}
void cxs_init(CXSEngine *e){
 memset(e,0,sizeof(*e));e->seed=0xB17C3E92u;
 for(int i=0;i<CXS_SEGMENTS;i++)e->d[i]=.125f;
 e->knob=(CXSParams){58,50,52,68,5,25,85,20,45,35,45,25,0};
 e->note=-1;e->velocity=127;e->stage=0;exchange(e);
}
void cxs_set(CXSEngine *e,const CXSParams *p){
 if(!p)return;
 e->knob=*p;
 #define LIM(name) e->knob.name=clampf(e->knob.name,0,100)
 LIM(pressure);LIM(balance);LIM(memory);LIM(density);LIM(attack);LIM(decay);LIM(sustain);LIM(release);
 LIM(cutoff);LIM(resonance);LIM(drive);LIM(lfo_rate);LIM(lfo_depth);
 #undef LIM
}
void cxs_note_on(CXSEngine *e,int midi_note,int velocity,int retrigger){
 if(midi_note<0||midi_note>127)return;
 int idx=-1;for(int i=0;i<e->held_count;i++)if(e->held[i]==midi_note){idx=i;break;}
 if(idx>=0){for(int i=idx;i+1<e->held_count;i++)e->held[i]=e->held[i+1];e->held_count--;}
 if(e->held_count<CXS_MAX_HELD)e->held[e->held_count++]=midi_note;
 if(retrigger||e->stage==0){e->env=0;e->phase=0;e->stage=1;}
 e->note=midi_note;e->freq=midi_hz(midi_note);e->velocity=velocity<1?1:velocity>127?127:velocity;e->gate=1;
}
void cxs_note_off(CXSEngine *e,int midi_note){
 for(int i=0;i<e->held_count;i++)if(e->held[i]==midi_note){for(int j=i;j+1<e->held_count;j++)e->held[j]=e->held[j+1];e->held_count--;break;}
 if(e->held_count){int n=e->held[e->held_count-1];e->note=n;e->freq=midi_hz(n);e->gate=1;}
 else{e->gate=0;e->stage=4;}
}
void cxs_panic(CXSEngine *e){e->held_count=0;e->gate=0;e->stage=0;e->env=0;e->note=-1;e->prev=0;e->low=e->band=0;}
static void envelope(CXSEngine *e){
 float *v=&e->env;float sr=CXS_SAMPLE_RATE;
 if(e->stage==1){float sec=.001f+2.5f*powf(e->knob.attack*.01f,2);*v+=1.0f/(sec*sr);if(*v>=1){*v=1;e->stage=2;}}
 if(e->stage==2){float sec=.002f+3.f*powf(e->knob.decay*.01f,2);float target=e->knob.sustain*.01f;*v-=(1-target)/(sec*sr);if(*v<=target){*v=target;e->stage=3;}}
 if(e->stage==3)*v=e->knob.sustain*.01f;
 if(e->stage==4){float sec=.004f+4.f*powf(e->knob.release*.01f,2);*v-=1.f/(sec*sr);if(*v<=0){*v=0;e->stage=0;}}
}
float cxs_tick(CXSEngine *e){
 if(e->stage==0)return 0;
 float inc=clampf(e->freq/CXS_SAMPLE_RATE,.000001f,.45f);
 e->phase+=inc;if(e->phase>=1){e->phase-=1;exchange(e);}
 float x=e->phase;
 int segment=0;for(int i=1;i<CXS_SEGMENTS;i++)if(x>=e->edge[i])segment=i;
 float raw=e->levels[segment];
 float mean=0;for(int i=0;i<CXS_SEGMENTS;i++)mean+=e->levels[i]*e->d[i];raw-=mean;
 /* PolyBLEP antialiasing of the eight transition events. */
 for(int i=0;i<CXS_SEGMENTS;i++){
  float before=e->levels[(i+7)&7],after=e->levels[i];
  float p=x-e->edge[i];if(p<0)p+=1;
  raw+=(after-before)*blep(p,inc);
 }
 float cut=35.f+8500.f*powf(e->knob.cutoff*.01f,2.2f);
 cut=fminf(cut,.42f*CXS_SAMPLE_RATE);
 /* Bounded 2-pole state-variable bandpass, nonlinear saturation after. */
 float f=2.f*sinf(CXS_PI*cut/CXS_SAMPLE_RATE);
 float q=.4f+2.2f*(1-e->knob.resonance*.01f);
 e->low+=f*e->band;
 float hi=raw-e->low-q*e->band;
 e->band=clampf(e->band+f*hi,-3,3);
 float bp=e->band;
 envelope(e);
 float drive=e->knob.drive*.01f;
 /* MOSFET-inspired asymmetric drive placeholder, not device-characterised. */
 float shape=tanhf(bp*(1+10*drive)+drive*.28f)-tanhf(drive*.28f);
 float value=shape*(.65f-.20f*drive)*e->env*(e->velocity/127.f);
 return clampf(value,-.95f,.95f);
}
int cxs_verify(const CXSEngine *e){
 float sum=0;
 for(int i=0;i<CXS_SEGMENTS;i++){
  if(!isfinite(e->d[i])||e->d[i]<.01999f||e->d[i]>.86f)return 0;
  sum+=e->d[i];
 }
 return fabsf(sum-1)<.0005f && isfinite(e->env)&&e->held_count>=0&&e->held_count<=CXS_MAX_HELD;
}
