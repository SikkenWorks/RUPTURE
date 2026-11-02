/* RUPTURE: one and only native 240x240 1-bit UI implementation.
 * SPDX-License-Identifier: GPL-3.0-only
 * Used identically by FM1 firmware AND the host PNG export. */
#include <stdint.h>
#include <stddef.h>
#include "rupture_help_cards.h"
#include "rupture_bug_qr.h"
#include "rupture_utrecht_art.h"
#include "rupture_eq2.h"
#include "rupture_eq2_tables.h"
#include "rupture_dam_sprites.h"
#include "rupture_dam_anim.h"
#include "rupture_crt_vcr.h"
#define RU_W 240
#define RU_H 240
#define RU_PAGE_PLAY 0
#define RU_PAGE_FILTER 1
#define RU_PAGE_ENV 2
#define RU_PAGE_LFO 3
#define RU_PAGE_FX 4
#define RU_PAGE_REVERB 5
#define RU_PAGE_DELAY 6
#define RU_PAGE_CHORUS 7
#define RU_PAGE_DRIVE 8
#define RU_PAGE_SEQ 9
#define RU_PAGE_SEQ_SLOTS 10
#define RU_PAGE_ARP 11
#define RU_PAGE_GUIDE 12
#define RU_PAGE_KEYLOCK 13
#define RU_PAGE_PROJECTS 14
#define RU_PAGE_GLO 15
#define RU_PAGE_SETTINGS 16
#define RU_PAGE_HELP 17
#define RU_PAGE_UPDATE 18
#define RU_PAGE_FOREIGN 19
#define RU_PAGE_FORMAT_CONFIRM 20
#define RU_PAGE_SAFE 21
#define RU_PAGE_NOTE_MEMORY 22
#define RU_PAGE_MIDI 23
#define RU_PAGE_LOADING 24
#define RU_PAGE_MENU 25
#define RU_PAGE_EQ 26
#define RU_PAGE_SHAPE 27
#define RU_PAGE_NAME 28
#define RU_PAGE_SUB1 29
#define RU_PAGE_SUB2 30
#define RU_PAGE_COMP 31
#define RU_PAGE_FOLDER 32
#define RU_PAGE_TRANSIENT 33
#define RU_PAGE_TILT 34
#define RU_PAGE_FUZZ 35
#define RU_PAGE_SANDPAPER 36
#define RU_PAGE_DIAG 37
#define RU_PAGE_STORAGE 38
#define RU_PAGE_STORAGE_PROJECTS 39
#define RU_PAGE_STORAGE_ACTIONS 40
#define RU_PAGE_STORAGE_SEQUENCES 41
#define RU_PAGE_STORAGE_SEQ_ACTIONS 42
#define RU_PAGE_WIPE_ALL 43
#define RU_PAGE_OTHER 44
#define RU_PAGE_ABOUT 45
#define RU_PAGE_BUG_REPORT 46
#define RU_PAGE_SAVER_OPTIONS 47
#define RU_PAGE_AUTOLOAD 48
#define RU_PAGE_COUNT 49
static const char *const ru_page_names[RU_PAGE_COUNT]={"PLAY","FILTER","ENVELOPE","LFO","FX","REVERB","DELAY","CHORUS","OVERDRIVE","SEQUENCER","SEQ SLOTS","ARPEGGIATOR","SCALE GUIDE","KEYLOCK","PROJECTS","GLO","SETTINGS","HELP","UPDATE","FOREIGN DATA","FORMAT CONFIRM","SAFE MODE","NOTE MEMORY","MIDI","LOADING","MENU","CHANNEL EQ","SOUND SHAPE","NAME EDITOR","SUB OSC 1","SUB OSC 2","COMPRESSOR","DISABLED","DISABLED","TILT EQ","OCTAVE FUZZ","SANDPAPER","AUDIO DIAGNOSTICS","STORAGE","PROJECTS","PROJECT ACTIONS","SEQUENCES","SEQ ACTIONS","WIPE ALL","OTHER","ABOUT","REPORT A BUG","SCREENSAVER","AUTO LOAD"};
typedef struct {uint8_t page,selected,project,seq_slot,root,mode,keylock,show_battery,usb_power;uint8_t boot_step;uint8_t dam_frame;int16_t battery;uint16_t k[4];uint16_t master;uint8_t seq_playing;uint16_t seq_current_step,seq_playhead,seq_length,seq_grid_mask,seq_hold_mask;uint8_t seq_sustain;uint8_t seq_note_count,seq_first_note,seq_bpm;uint8_t seq_status;uint8_t seq_present;uint8_t active_project,active_seq_slot;uint8_t project_status;uint8_t note_count;uint8_t format_yes;uint8_t help_page,help_scroll;uint8_t filter;uint8_t volume;uint8_t fx_on;uint8_t fx_selected;uint8_t eq_on,eq_bypassed;uint8_t eq_mode,eq_tab,eq_freq[4],eq_q[2],eq_gain_index[4],eq_curve[54];uint8_t sub_blend;uint8_t macro[4];uint8_t animate;uint8_t knob_visual[4];int8_t octave;uint16_t project_mask,seq_mask;char project_names[12][13],seq_names[8][13];char edit_name[13];uint8_t edit_cursor,edit_is_sequence;int8_t store_error;
 uint8_t audio_fault,settings_open,auto_save,auto_choice,boot_choice,saver_active,saver_frame,main_wave,saver_enabled,saver_delay_index,saver_options_selected;
 uint8_t crt_enabled;
 uint8_t manager_project,manager_seq,manager_action,manager_confirm,manager_status;
 uint8_t manager_seq_mask,manager_return,about_scroll;
 char manager_seq_names[8][13];
 uint16_t diag_wall,diag_dsp,diag_render,diag_copy,diag_slack;
 uint32_t diag_late,diag_guard,diag_scope;uint16_t ota_error;} RuState;
#ifdef RU_BITS_EXTERNAL
/* Display scratch is unused while drawing RUPTURE. */
#define ru_bits ((uint8_t*)etch_scope_canvas)
#else
static uint8_t ru_bits[(RU_W*RU_H)/8];
#endif
static const char ru_font_chars[]="ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 -/:?.%=><#+,'()!";
static const uint8_t ru_font[][7]={
 {14,17,17,31,17,17,17} /* A */,
 {30,17,17,30,17,17,30} /* B */,
 {15,16,16,16,16,16,15} /* C */,
 {30,17,17,17,17,17,30} /* D */,
 {31,16,16,30,16,16,31} /* E */,
 {31,16,16,30,16,16,16} /* F */,
 {15,16,16,23,17,17,15} /* G */,
 {17,17,17,31,17,17,17} /* H */,
 {31,4,4,4,4,4,31} /* I */,
 {7,2,2,2,18,18,12} /* J */,
 {17,18,20,24,20,18,17} /* K */,
 {16,16,16,16,16,16,31} /* L */,
 {17,27,21,21,17,17,17} /* M */,
 {17,25,21,19,17,17,17} /* N */,
 {14,17,17,17,17,17,14} /* O */,
 {30,17,17,30,16,16,16} /* P */,
 {14,17,17,17,21,18,13} /* Q */,
 {30,17,17,30,20,18,17} /* R */,
 {15,16,16,14,1,1,30} /* S */,
 {31,4,4,4,4,4,4} /* T */,
 {17,17,17,17,17,17,14} /* U */,
 {17,17,17,17,17,10,4} /* V */,
 {17,17,17,21,21,27,17} /* W */,
 {17,17,10,4,10,17,17} /* X */,
 {17,17,10,4,4,4,4} /* Y */,
 {31,1,2,4,8,16,31} /* Z */,
 {14,19,21,21,25,17,14} /* 0 */,
 {4,12,4,4,4,4,14} /* 1 */,
 {14,17,1,2,4,8,31} /* 2 */,
 {30,1,1,14,1,1,30} /* 3 */,
 {2,6,10,18,31,2,2} /* 4 */,
 {31,16,16,30,1,1,30} /* 5 */,
 {15,16,16,30,17,17,14} /* 6 */,
 {31,1,2,4,8,8,8} /* 7 */,
 {14,17,17,14,17,17,14} /* 8 */,
 {14,17,17,15,1,1,30} /* 9 */,
 {0,0,0,0,0,0,0} /* space */,
 {0,0,0,31,0,0,0} /* - */,
 {1,1,2,4,8,16,16} /* / */,
 {0,4,4,0,4,4,0} /* : */,
 {14,17,1,2,4,0,4} /* ? */,
 {0,0,0,0,0,4,4} /* . */,
 {25,25,2,4,8,19,19} /* % */,
 {0,31,0,31,0,0,0} /* = */,
 {16,8,4,2,4,8,16} /* > */,
 {1,2,4,8,4,2,1} /* < */,
 {10,31,10,31,10,0,0} /* # */,
 {0,4,4,31,4,4,0} /* + */,
 {0,0,0,0,4,4,8} /* , */,
 {4,4,8,0,0,0,0} /* apostrophe */,
 {2,4,8,8,8,4,2} /* ( */,
 {8,4,2,2,2,4,8} /* ) */,
 {4,4,4,4,4,0,4} /* ! */
};
static void ru_clear(void){for(uint32_t i=0;i<((RU_W*RU_H)/8u);i++)ru_bits[i]=0;}
static void ru_set(int x,int y){if((unsigned)x>=240u||(unsigned)y>=240u)return;uint32_t i=(uint32_t)y*240u+(uint32_t)x;ru_bits[i>>3]|=(uint8_t)(0x80u>>(i&7));}
static int ru_get(int x,int y){if((unsigned)x>=240u||(unsigned)y>=240u)return 0;uint32_t i=(uint32_t)y*240u+(uint32_t)x;return !!(ru_bits[i>>3]&(0x80u>>(i&7)));}
static void ru_rect(int x,int y,int w,int h){for(int yy=y;yy<y+h;yy++)for(int xx=x;xx<x+w;xx++)ru_set(xx,yy);}
static void ru_frame(int x,int y,int w,int h){ru_rect(x,y,w,1);ru_rect(x,y+h-1,w,1);ru_rect(x,y,1,h);ru_rect(x+w-1,y,1,h);}
static void ru_dither(int x,int y,int w,int h,int tight){for(int j=y;j<y+h;j++)for(int i=x;i<x+w;i++)if(tight?((i+j)&1)==0:((i+3*j)&3)==0)ru_set(i,j);}
static void ru_line(int x0,int y0,int x1,int y1){int dx=x1>x0?x1-x0:x0-x1,dy=y1>y0?y0-y1:y1-y0;int sx=x0<x1?1:-1,sy=y0<y1?1:-1,err=dx+dy;for(;;){ru_set(x0,y0);if(x0==x1&&y0==y1)break;int e=err*2;if(e>=dy){err+=dy;x0+=sx;}if(e<=dx){err+=dx;y0+=sy;}}}
static void ru_text(const char *s,int x,int y,int scale){for(int c=0;s&&s[c]&&x<237;c++,x+=6*scale){char a=s[c];if(a>='a'&&a<='z')a-=32;uint32_t g=0;for(;ru_font_chars[g]&&ru_font_chars[g]!=a;g++){ /* lookup glyph */ }if(!ru_font_chars[g])continue;for(int r=0;r<7;r++)for(int j=0;j<5;j++)if(ru_font[g][r]&(16>>j))ru_rect(x+j*scale,y+r*scale,scale,scale);}}
static int ru_strlen(const char *s){int n=0;while(s&&s[n])n++;return n;}
static void ru_center(const char *s,int x,int y,int scale){ru_text(s,x-(ru_strlen(s)*6*scale)/2,y,scale);}
static void ru_uint(uint32_t v,char *s){char r[12];int n=0,i=0;do{r[n++]=(char)('0'+v%10u);v/=10u;}while(v&&n<10);while(n)s[i++]=r[--n];s[i]=0;}
static void ru_battery(int16_t value,uint8_t usb_power){ru_frame(203,8,26,12);ru_rect(229,11,3,6);if(value<0){ru_line(206,12,224,17);}else{if(value>100)value=100;ru_rect(204,9,(value*24+50)/100,10);} if(usb_power){/* 1-bit lightning bolt to the left of the battery */ru_line(195,6,190,13);ru_line(190,13,195,13);ru_line(195,13,192,21);ru_line(192,21,199,11);ru_line(199,11,194,11);}}
/* Dedicated heavyweight 8x9 pixel wordmark, not the generic menu font.
 * Every pixel is binary: no grayscale and no antialiasing. */
static void ru_wordmark(void){
 static const uint8_t glyph[5][9]={
  {252,198,198,252,216,204,198,198,198}, /* R */
  {198,198,198,198,198,198,198,198,124}, /* U */
  {252,198,198,252,192,192,192,192,192}, /* P */
  {255,255,24,24,24,24,24,24,24},       /* T */
  {254,192,192,252,192,192,192,192,254} /* E */
 };
 static const uint8_t spelling[7]={0,1,2,3,1,0,4};
 for(int c=0;c<7;c++)for(int y=0;y<9;y++)for(int x=0;x<8;x++){
  int px=7+c*18+x*2,py=3+y*2;
  /* A crisp 1-bit tear cuts diagonally through all seven header letters.
   * Carve two pixels in negative space, not an overlay obscuring the brand. */
  int crack_y=9+(px/18)%3+((px/9)&1)*2;
  if(!(c==6 && y==3) && py<=crack_y+1 && py+1>=crack_y-1)continue;
  if(glyph[spelling[c]][y]&(0x80u>>x))ru_rect(px,py,2,2);
 }
}
static void ru_header(const char *s,const RuState *v){ru_wordmark();if(v->show_battery)ru_battery(v->battery,v->usb_power);ru_rect(7,25,226,1);ru_text(s,7,32,1);
 /* Oscillator belongs to the header, never to the waveform/knob area.
  * 93..148 is clear after PLAY/SOUND SHAPE and before OCT at x=166. */
 if(v->page==RU_PAGE_PLAY || v->page==RU_PAGE_SHAPE)
  ru_text(v->main_wave?"SAWTOOTH":"CXS",93,32,1);
 {char oct[7]={'O','C','T',' ',0,0,0};
  if(v->octave==0){oct[4]='0';}
  else{oct[4]=v->octave<0?'-':'+';
   oct[5]=(char)('0'+(v->octave<0?-v->octave:v->octave));}
  ru_text(oct,166,32,1);}
 ru_rect(7,45,226,1);}
static uint8_t ru_footer_status=0u;
/* Project/sequence progress overlay. Percent tracks completed flash I/O stages. */
static void ru_storage_popup(uint8_t kind,uint8_t stage,uint8_t pct,uint8_t result){
 ru_clear();
 ru_wordmark();ru_frame(12,69,216,111);
 ru_center(kind==1u?"PROJECT SAVE":kind==2u?"PROJECT LOAD":kind==3u?"SEQUENCE SAVE":"SEQUENCE LOAD",120,82,1);
 const char *label=result==1u?"COMPLETE":result==2u?"SAVE FAILED":
  stage==1u?"ERASING FLASH":stage==2u?"WRITING FLASH":
  stage==3u?"VERIFYING CRC":stage==4u?"READING DATA":"PREPARING";
 ru_center(label,120,110,1);
 ru_frame(25,133,190,15);
 uint32_t width=(uint32_t)pct*186u/100u;if(width>186u)width=186u;
 if(width)ru_dither(27,135,(int)width,11,1);
 char text[12];ru_uint(pct,text);ru_center(text,120,154,1);
}
static void ru_footer(const char *s){
 ru_rect(7,226,226,1);
 if(ru_footer_status==1u)ru_center("PROJECT SAVED",120,231,1);
 else if(ru_footer_status==2u)ru_center("SAVE FAILED",120,231,1);
 else if(ru_footer_status==3u)ru_center("AUTO SAVED",120,231,1);
 else ru_center(s,120,231,1);
}
static const int8_t ru_needle_xy[101][2]={
 {-11,11},
 {-11,10},
 {-12,10},
 {-12,9},
 {-12,8},
 {-13,8},
 {-13,7},
 {-13,7},
 {-14,6},
 {-14,5},
 {-14,5},
 {-14,4},
 {-15,3},
 {-15,3},
 {-15,2},
 {-15,1},
 {-15,0},
 {-15,0},
 {-15,-1},
 {-15,-2},
 {-15,-2},
 {-15,-3},
 {-15,-4},
 {-14,-4},
 {-14,-5},
 {-14,-6},
 {-14,-6},
 {-13,-7},
 {-13,-8},
 {-13,-8},
 {-12,-9},
 {-12,-9},
 {-11,-10},
 {-11,-10},
 {-10,-11},
 {-10,-11},
 {-9,-12},
 {-9,-12},
 {-8,-13},
 {-7,-13},
 {-7,-13},
 {-6,-14},
 {-6,-14},
 {-5,-14},
 {-4,-14},
 {-4,-15},
 {-3,-15},
 {-2,-15},
 {-1,-15},
 {-1,-15},
 {0,-15},
 {1,-15},
 {1,-15},
 {2,-15},
 {3,-15},
 {4,-15},
 {4,-14},
 {5,-14},
 {6,-14},
 {6,-14},
 {7,-13},
 {7,-13},
 {8,-13},
 {9,-12},
 {9,-12},
 {10,-11},
 {10,-11},
 {11,-10},
 {11,-10},
 {12,-9},
 {12,-9},
 {13,-8},
 {13,-8},
 {13,-7},
 {14,-6},
 {14,-6},
 {14,-5},
 {14,-4},
 {15,-4},
 {15,-3},
 {15,-2},
 {15,-2},
 {15,-1},
 {15,0},
 {15,0},
 {15,1},
 {15,2},
 {15,3},
 {15,3},
 {14,4},
 {14,5},
 {14,5},
 {14,6},
 {13,7},
 {13,7},
 {13,8},
 {12,8},
 {12,9},
 {12,10},
 {11,10},
 {11,11}
};
static void ru_knob(int x,int y,uint32_t v){for(int yy=-22;yy<=22;yy++)for(int xx=-22;xx<=22;xx++){int r2=xx*xx+yy*yy;if((r2>=400&&r2<=484)|| (r2>=272&&r2<=300&&((xx+yy)&1)==0))ru_set(x+xx,y+yy);}if(v>100u)v=100u;int px=ru_needle_xy[v][0],py=ru_needle_xy[v][1];ru_line(x,y,x+px,y+py);ru_rect(x-1,y-1,3,3);}
static void ru_metric(int x,int y,const char *name,const char *value,uint32_t knob){ru_knob(x,y,knob);ru_center(name,x,y+26,1);ru_center(value,x,y+37,1);}
static const char *const ru_notes[]={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
/* # unsupported in glyph? supported directly. */
static const char *const ru_scale[]={"MAJOR","DORIAN","PHRYGIAN","LYDIAN","MIXOLYDIAN","MINOR","LOCRIAN","HARM MIN","MELOD MIN"};
static const char *const ru_arp[]={"OFF","UP","DOWN","UPDOWN","ORDER","RANDOM","CUSTOM","RIFF"};
static uint8_t ru_knob_normal(const RuState *v,int i){
 uint32_t val=v->k[i];
 switch(v->page){
 case RU_PAGE_SETTINGS: if(i==0)val=val/2u;break;
 case RU_PAGE_SHAPE:break;
 case RU_PAGE_SUB1:case RU_PAGE_SUB2:
   if(i==0)val=val*25u;else if(i==1)val=val*100u/3u;break;
 case RU_PAGE_FX:if(v->fx_selected==0u&&i==0)val=val*50u;else if((v->fx_selected==1u||v->fx_selected==2u)&&i==2)val=val*100u;else if(v->fx_selected==3u&&i==3)val=val*25u;break;
 case RU_PAGE_EQ:if(i<3)val=val*100u/200u;else val=val/2u;break;
 case RU_PAGE_ENV:if(i==0||i==1||i==3)val=val/100u;break;
 case RU_PAGE_LFO:if(i==0)val=(v->k[2]==1u)?(val/2u):(val*100u/11u);else if(i==2)val=val*50u;else if(i==3)val=val*100u;break;
 case RU_PAGE_SEQ:if(i==0){uint16_t len[]={4,8,16,32,64,128};uint8_t z=0;while(z<5 && len[z]!=val)z++;val=z*20u;}else if(i==1)val=(val-30u)*100u/210u;break;
 case RU_PAGE_NOTE_MEMORY:if(i==0)val=val*50u;else if(i==1)val=val*100u;else if(i==2)val=val/4u;else val=val*100u/12u;break;
 case RU_PAGE_FILTER:if(i==0)val=val*25u;break;
 case RU_PAGE_ARP:if(i==0||i==1)val=val*100u/7u;
  else if(i==2)val=(val<=1u?0u:(val-1u)*100u/11u);
  break;
 case RU_PAGE_GUIDE:case RU_PAGE_KEYLOCK:
  if(i==0)val=val*100u/11u;
  else if(i==1)val=val*100u/8u;
  else val=val*50u;
  break;
 case RU_PAGE_PROJECTS:if(i==0)val=(val<=1u?0u:(val-1u)*100u/11u);break;
 case RU_PAGE_SEQ_SLOTS:if(i==0)val=(val<=1u?0u:(val-1u)*100u/7u);break;
 case RU_PAGE_REVERB:if(i==0)val=val*50u;break;
 case RU_PAGE_DELAY:case RU_PAGE_CHORUS:if(i==2)val=val*100u;break;
 case RU_PAGE_DRIVE:if(i==3)val=val*25u;break;
 default:break;
 }
 return (uint8_t)(val>100u?100u:val);
}
static const char *const ru_knob_labels[RU_PAGE_COUNT][4]={
 {"PRESSURE","BALANCE","MEMORY","DENSITY"},{"TYPE","CUTOFF","RESONANCE","DRIVE"},{"ATTACK","DECAY","SUSTAIN","RELEASE"},{"RATE","DEPTH","MODE","TARGET"},{"REVERB","DELAY","CHORUS","MOSFET"},{"TYPE","DECAY","SIZE","MIX"},{"TIME","FEEDBACK","ROUTING","MIX"},{"RATE","DEPTH","ROUTING","MIX"},{"DRIVE","CUTOFF","RESONANCE","TYPE"},
 {"LENGTH","BPM","VELOCITY","GATE"},{"SLOT","ACTION","RATE","SWING"},{"MODE","RATE","OCTAVE","GATE"},{"ROOT","SCALE","GUIDE","CHORD"},{"ROOT","SCALE","KEYLOCK","CHORD"},{"PROJECT","ACTION","NAME","CONFIRM"},
 {"MENU","SELECT","ENTER","HOME"},{"MASTER","MIDI","TUNING","BACK"},{"CHAPTER","PAGE","SCROLL",""},{"CHECK","LOADER","VERIFY","BACK"},{"NO","YES","FORMAT","BACK"},{"NO","YES","CONFIRM","BACK"},{"MIDI","USB","TRS","RECOVERY"},{"PRIORITY","LEGATO","GLIDE","BEND"},{"USB","TRS","CHANNEL","CLOCK"},{"","","",""},{"","","",""},{"LOW","MID","HIGH","AIR +/-"},{"WEIGHT","EDGE","BITE","PUNCH"},
 {"CURSOR","CHAR","SAVE","BACK"}, /* NAME EDITOR index 28 */
 {"TYPE","OCTAVE","DETUNE","DRIFT"},
 {"TYPE","OCTAVE","DETUNE","DRIFT"},
 {"THRESH","RATIO","ATTACK","RELEASE"},
 {"FOLD","SYMMETRY","TONE","MIX"},
 {"ATTACK","SUSTAIN","SENS","MIX"},
 {"TILT","FREQ","MIX","OUTPUT"},
 {"DRIVE","OCTAVE","TONE","MIX"},
 {"SMOOTH","SOFT","WARMTH","MIX"}
 };
static void ru_draw_knobs(const RuState *v){if(v->page>=RU_PAGE_COUNT||v->page==RU_PAGE_LOADING||v->page==RU_PAGE_MENU)return;char buf[4][12];for(int i=0;i<4;i++){if((v->page==RU_PAGE_HELP&&i==3)||(v->page==RU_PAGE_SETTINGS&&i>0))continue;ru_uint(v->k[i],buf[i]);const char *txt=buf[i];
 if((v->page==RU_PAGE_GUIDE||v->page==RU_PAGE_KEYLOCK)&&i==0)txt=ru_notes[v->root%12];
 if((v->page==RU_PAGE_GUIDE||v->page==RU_PAGE_KEYLOCK)&&i==1)txt=ru_scale[v->mode%9];
 if(v->page==RU_PAGE_KEYLOCK&&i==2){static const char *const lock[]={"OFF","GUIDE","LOCK"};txt=lock[v->keylock%3];}
 if(v->page==RU_PAGE_EQ){if(i<3){ru_uint(v->k[i],buf[i]);int l=ru_strlen(buf[i]);if(l<10){buf[i][l]='%';buf[i][l+1]=0;}}else{
  int air=(int)v->k[i]-100;
  buf[i][0]=(air<0?'-':'+');if(air<0)air=-air;
  ru_uint((uint32_t)air,buf[i]+1);
 }txt=buf[i];}
 if(v->page==RU_PAGE_SETTINGS&&i==0){ru_uint(v->master,buf[i]);int q=ru_strlen(buf[i]);if(q<10){buf[i][q]='%';buf[i][q+1]=0;}txt=buf[i];}
 if((v->page==RU_PAGE_GUIDE||v->page==RU_PAGE_KEYLOCK)&&i==3){static const char *const chord[]={"OFF","TRIAD","SEVENTH"};txt=chord[v->k[3]%3];}
 if((v->page==RU_PAGE_SUB1||v->page==RU_PAGE_SUB2)&&i==0){static const char *const kinds[]={"OFF","CXS","CLASSIC","SQUARE","SAW"};txt=kinds[v->k[0]%5u];}
 if((v->page==RU_PAGE_SUB1||v->page==RU_PAGE_SUB2)&&i==1){static const char *const octs[]={"-2","-1","0","+1"};txt=octs[v->k[1]%4u];}
 if((v->page==RU_PAGE_SUB1||v->page==RU_PAGE_SUB2)&&i==2){int n=(int)v->k[2]-50; char *t=buf[i];int off=0;if(n<0){t[off++]='-';n=-n;}else t[off++]='+';char digs[8];ru_uint((uint32_t)n,digs);for(int z=0;digs[z];z++)t[off++]=digs[z];t[off++]='C';t[off]=0;txt=t;}
 if(v->page==RU_PAGE_NOTE_MEMORY&&i==0){static const char *const pr[]={"LAST","HIGH","LOW"};txt=pr[v->k[0]%3];}
 if(v->page==RU_PAGE_NOTE_MEMORY&&i==1)txt=v->k[1]?"ON":"OFF";
 if(v->page==RU_PAGE_ARP&&i==0)txt=ru_arp[v->k[0]%8];
 if(v->page==RU_PAGE_LFO&&i==2){static const char *const modes[]={"KEYED","FREE","TEMPO"};txt=modes[v->k[2]%3u];}
 if(v->page==RU_PAGE_LFO&&i==3)txt=v->k[3]?"LEVEL":"PITCH";
 if((v->page==RU_PAGE_REVERB||(v->page==RU_PAGE_FX&&v->fx_selected==0u))&&i==0){static const char *const types[]={"SPRING","HALL","ROOM"};txt=types[v->k[0]%3];}
 if(v->page==RU_PAGE_FILTER&&i==0){static const char *const filt[]={"OFF","LOWPASS","BANDPASS","HIGHPASS","LADDER"};txt=filt[v->filter%5];}
 if(v->page==RU_PAGE_GLO){static const char *const gl[]={"SETTINGS","HELP","SEL","HOME"};txt=gl[i];}
 if(v->page==RU_PAGE_HELP){if(i==3)txt="";}
 if(v->page==RU_PAGE_PROJECTS&&i==0){ru_uint(v->project,buf[i]);txt=buf[i];}
 if(v->page==RU_PAGE_PROJECTS&&i==1)txt=v->k[1]?"SAVE":"LOAD";
 if(v->page==RU_PAGE_SEQ_SLOTS&&i==0){ru_uint(v->seq_slot,buf[i]);txt=buf[i];}
 if(v->page==RU_PAGE_SEQ_SLOTS&&i==1)txt=v->k[1]?"SAVE":"LOAD";
 int x=v->page==RU_PAGE_SETTINGS?120:((i%2)?176:64),
 y=v->page==RU_PAGE_SETTINGS?123:((i/2)?181:105);ru_knob(x,y,v->animate?v->knob_visual[i]:ru_knob_normal(v,i));ru_center((v->page==RU_PAGE_PLAY && v->main_wave)?
   (i==0?"BRIGHT":i==1?"EDGE":i==2?"BODY":"LEVEL"):
   ru_knob_labels[v->page==RU_PAGE_FX?(v->fx_selected<4u?(RU_PAGE_REVERB+v->fx_selected):v->fx_selected==4u?RU_PAGE_COMP:v->fx_selected==5u?RU_PAGE_TILT:v->fx_selected==6u?RU_PAGE_FUZZ:RU_PAGE_SANDPAPER):v->page][i],x,y+26,1);if(ru_strlen(txt)<=10)ru_center(txt,x,y+36,1);else {char truncated[11];int q=0;while(q<10&&txt[q]){truncated[q]=txt[q];q++;}truncated[q]=0;ru_center(truncated,x,y+36,1);} }
}
/* Visual response approximates the actual fixed-cross-over 3-band + HIGH CUT DSP.
 * LOW 200 Hz, HIGH 6 kHz; peaks are not parametric until the DSP supports it. */
static void ru_eq_curve(const RuState *v){
 if(v->eq_mode){
  static const char *names[]={"LOW","BODY","BITE","AIR"};
  static const char *tabs[]={"GAIN +/- 9DB","FREQUENCY","PEAK WIDTH (Q)"};
  ru_center("CHANNEL EQ 2.0",120,50,1);
  ru_center(tabs[v->eq_tab%3u],120,64,1);
  ru_frame(12,76,216,83);
  for(int y=88;y<158;y+=14)ru_dither(13,y,214,1,0);
  /* Display the actual RBJ biquad magnitude response computed from the
   * coefficients published to the post-mix DSP. 54 logarithmic points. */
  int previous=119;
  for(int x=14;x<226;x++){
   int i=(x-14)*53/211;
   int y=v->eq_curve[i];if(y<80||y>157)y=119;
   if(x>14)ru_line(x-1,previous,x,y);
   previous=y;
  }
  for(int i=0;i<4;i++){
   int x=(i%2)?130:13,y=i<2?168:192;
   char n[12];const char *val=n;
   if(v->eq_tab==0){
    int halfdb=(int)v->eq_gain_index[i]-18;
    int mag=halfdb<0?-halfdb:halfdb;
    n[0]=halfdb<0?'-':'+';
    char z[8];ru_uint((uint32_t)mag/2u,z);
    int off=1;for(int k=0;z[k]&&off<7;k++)n[off++]=z[k];
    n[off++]='.';n[off++]=(mag&1)?'5':'0';n[off]=0;
   }else if(v->eq_tab==1){
    ru_uint(ru_eq2_freq[i][v->eq_freq[i]<=100u?v->eq_freq[i]:100u],n);
   }else if(i==1||i==2){
    uint32_t q10=3u+((uint32_t)v->eq_q[i-1]*77u+50u)/100u;
    char z[8];ru_uint(q10/10u,z);n[0]=z[0];n[1]='.';n[2]=(char)('0'+q10%10u);n[3]=0;
   }else val="FIXED";
   ru_text(names[i],x,y,1);ru_text(val,x+55,y,1);
  }
  ru_footer("EDIT NEXT / ALG LEGACY");return;
 }
 ru_center("LEGACY CHANNEL EQ",120,51,1);
 ru_frame(11,65,218,80);
 for(int y=78;y<145;y+=14)ru_dither(12,y,216,1,0);
 for(int x=39;x<228;x+=36)ru_dither(x,67,1,76,0);
 int oldy=105;
 for(int x=13;x<=225;x++){
  int32_t pos=x-13; /* log-like horizontal band interpolation */
  int32_t p0=(int32_t)v->k[0]-100,p1=(int32_t)v->k[1]-100,p2=(int32_t)v->k[2]-100;
  int32_t gain;
  if(pos<76)gain=p0+(p1-p0)*pos/76;
  else if(pos<158)gain=p1+(p2-p1)*(pos-76)/82;
  else gain=p2;
  /* K4 bipolar shelf: boosts OR attenuates high band. */
  int32_t air=(int32_t)v->k[3]-100;
  int32_t lift=pos>145?air*(pos-145)*22/(100*67):0;
  int y=105-gain*25/100-(int)lift;
  if(y<68)y=68;
  if(y>141)y=141;
  if(x>13)ru_line(x-1,oldy,x,y);
  oldy=y;
 }
 ru_text("20",13,148,1);ru_text("200",64,148,1);
 ru_text("2K",139,148,1);ru_text("20K",197,148,1);
 ru_center(v->eq_bypassed?"EQ OFF / K1 ENABLE":v->eq_on?"EQ ACTIVE / K4 +/- AIR":"EQ FLAT / K4 NEUTRAL",120,161,1);
 /* four compact values below graph, without knobs obscuring the curve */
 static const char *const lab[]={"LOW","MID","HIGH","AIR +/-"};
 for(int i=0;i<4;i++){
  int x=(i%2)?138:14,y=(i/2)?195:171;
  ru_text(lab[i],x,y,1);
  char val[12];ru_uint(v->k[i],val);ru_text(val,x+76,y,1);
 }
 ru_footer("ALGORITHM EQ2 / HOME BACK");
}
/* Sequencer pixels and firmware share this exact 1-bit renderer. Pages of 16
   steps keep a 128-step song legible on the 240x240 panel. */
static void ru_seq_screen(const RuState *v){
 char a[12],b[12];
 ru_text("LEN",13,52,1);ru_uint(v->seq_length,a);ru_text(a,40,52,1);
 ru_text("BPM",113,52,1);ru_uint(v->seq_bpm,b);ru_text(b,145,52,1);
 ru_text(v->seq_playing?"PLAY":"EDIT",198,52,1);
 uint16_t display=v->seq_playing?v->seq_playhead:v->seq_current_step;
 uint16_t first=(uint16_t)(display&~15u);
 int visible=v->seq_length<16u?(int)v->seq_length:16;
 for(int n=0;n<visible;n++){
  int x=12+(n%8)*27,y=70+(n/8)*29;
  if(visible<=8)x+=((8-visible)*27)/2;
  ru_frame(x,y,24,23);
  if(v->seq_grid_mask&(1u<<n))ru_dither(x+3,y+3,18,17,1);
  else if(v->seq_hold_mask&(1u<<n)){ru_rect(x+3,y+10,18,3);}
  if(first+(uint16_t)n==v->seq_current_step){ru_frame(x-2,y-2,28,27);}
  if(v->seq_playing && first+(uint16_t)n==v->seq_playhead){ru_rect(x+8,y+9,8,5);}
 }
 char step[12],total[12];ru_uint(v->seq_current_step+1u,step);ru_uint(v->seq_length,total);
 ru_text("STEP",14,133,1);ru_text(step,47,133,1);ru_text("/",76,133,1);ru_text(total,90,133,1);
 if(v->seq_note_count){
  ru_text(ru_notes[v->seq_first_note%12],156,133,1);
  /* Pitch octave and HOLD count share one truly centered value column.
   * This keeps single- and multi-digit values visually aligned on FM1. */
  ru_uint(v->seq_first_note/12u-1u,a);ru_center(a,180,133,1);
  if(v->seq_note_count>1u){ru_text("+",196,133,1);ru_uint(v->seq_note_count-1u,b);ru_text(b,205,133,1);}
 }else ru_text("REST",169,133,1);
 if(v->seq_note_count){
  char hold[12];ru_uint(v->seq_sustain,hold);ru_text("HOLD",130,145,1);ru_center(hold,180,145,1);
 }else ru_center("SELECT STEP  KEYS ADD/REMOVE",120,144,1);
 const char *names[4]={"LENGTH","BPM","VELOCITY","GATE"};
 for(int i=0;i<4;i++){
  int x=36+i*56;
  ru_knob(x,178,v->animate?v->knob_visual[i]:ru_knob_normal(v,i));
  ru_center(names[i],x,202,1);
  ru_uint(v->k[i],a);ru_center(a,x,213,1);
 }
 ru_footer("ALG HOLD   SAVE SEQUENCES");
}
/* V1.20: ROM 1-bit tear animation, 240x240 with no extra screen memory.
 * A single immutable final artwork is exposed through a moving jagged slit,
 * so successive frames cannot jump between unrelated crack illustrations.
 * Never called from audio IRQ. All drawing stays in existing ru_bits canvas. */
#include "rupture_splash_bitmap.h"
#define RU_SPLASH_STEPS 24u
static int ru_tear_axis(int x){
 return 120+(int)((((unsigned)x/9u)*7u+(((unsigned)x)/23u)*3u)%15u)-7;
}
static void ru_splash_crack(int reach){
 int last=ru_tear_axis(0);
 for(int x=1;x<reach&&x<240;x++){
  int y=ru_tear_axis(x);
  ru_line(x-1,last,x,y);
  if((x%53)==17 && x+7<reach)ru_line(x,y,x+6,y-9);
  if((x%71)==39 && x+5<reach)ru_line(x,y,x+4,y+10);
  last=y;
 }
}
static void ru_brand(const RuState *v){
 uint32_t step=v->boot_step;
 ru_clear();
 if(step>=RU_SPLASH_STEPS){
  for(uint32_t i=0;i<7200u;i++)ru_bits[i]=ru_splash_final_bitmap[i];
  return;
 }
 if(step<=8u){
  if(step)ru_splash_crack((int)((step*240u)/8u));
  return;
 }
 uint32_t k=step-8u;
 int radius=(int)((k*k*141u)/256u);
 for(int y=0;y<240;y++){
  const uint32_t row=(uint32_t)y*240u;
  for(int x=0;x<240;x++){
   /* A different stepped contour per column makes the crack split
    * like torn fabric rather than two perfectly straight shutters. */
   int ragged=(int)((((unsigned)x*17u)+((unsigned)x/7u)*43u)%19u)-9;
   int dy=y-ru_tear_axis(x);if(dy<0)dy=-dy;
   if(dy>radius+ragged)continue;
   uint32_t p=row+(uint32_t)x;
   if(ru_splash_final_bitmap[p>>3]&(0x80u>>(p&7)))
    ru_bits[p>>3]|=(uint8_t)(0x80u>>(p&7));
  }
 }
 /* Preserve the initial hairline while the rupture is still tiny. */
 if(radius<8)ru_splash_crack(240);
}
/* V1.30: pixel meadow fracturing into a canyon. Visual work is exclusively
 * in the UI loop. No DSP, additional framebuffer, floats, heap or random IO.
 * A single deterministic frame reaches the final hold state at frame 63. */
static void ru_clr(int x,int y){if((unsigned)x>=240u||(unsigned)y>=240u)return;uint32_t i=(uint32_t)y*240u+(uint32_t)x;ru_bits[i>>3]&=(uint8_t)~(0x80u>>(i&7));}
/* V1.50: Utrecht screensaver rupture extended through the street, behind the
 * front houses and up through the Dom Tower. One cracked tower half sinks a
 * few pixels at the peak of the animation. The saver still uses no extra full
 * screen framebuffers or sprite sheets: everything is drawn procedurally in the
 * existing UI loop and then reversed by the established saver state machine. */
static uint32_t ru_land_hash(uint32_t x,uint32_t y){
 uint32_t n=x*0x45d9f3bu+y*0x9e3779b9u+0x1f123bb5u;
 n^=n>>15;n*=0x2c1b3c6du;return n^(n>>12);
}
static int ru_city_pixel(int x,int y){
 if((unsigned)x>=240u||(unsigned)y>=RU_UTRECHT_HEIGHT)return 0;
 return !!(ru_utrecht_city[(uint32_t)y*30u+((uint32_t)x>>3)]&(uint8_t)(128u>>(x&7)));
}
static int ru_tower_center(int y){
 return y<80?121:(y<130?120:119);
}
static int ru_tower_sink_region(int x,int y){
 if(y<3||y>118||x<110||x>138||!ru_city_pixel(x,y))return 0;
 return x>=ru_tower_center(y);
}
static int ru_city_fault_x(int y){
 if(y>=171){
  int ry=y-171;
  return 119+((ry/11)%4-1)*3;
 }
 if(y>=145)return 118+((170-y)/8)%2-((170-y)/13)%2;
 if(y>=115)return 118+((145-y)/10);
 if(y>=80)return 119+(((115-y)/9)&1);
 if(y>=50)return 120+(((80-y)/8)%2);
 if(y>=20)return 121-(((50-y)/9)%2);
 return 122;
}
static int ru_crack_hidden_by_houses(int x,int y){
 return y>=118&&y<=156&&x>=105&&x<=136;
}
/* Cityscape directly sourced from the author's monochrome Utrecht art.
 * The foreground street fractures first; then the rupture threads upward
 * behind the front houses and finally tears through the Dom Tower itself. */
static const uint16_t ru_saver_time_values[8]={15u,30u,45u,60u,90u,120u,180u,300u};
static void ru_crack_saver(uint8_t frame){
 int phase=frame<6u?0:(int)frame-6;
 int depth=phase>47?47:phase;
 int opening=depth>0?(1+depth*75/47):0;
 int tower_sink=0;
 if(depth>40){
  tower_sink=1+((depth-41)*4)/6;
  if(tower_sink>5)tower_sink=5;
 }
 /* Draw the static Utrecht skyline, but keep the moving tower half back so it
  * can be re-plotted a few pixels lower once the crack reaches the tower. */
 for(int y=0;y<RU_UTRECHT_HEIGHT;y++)for(int x=0;x<240;x++){
  if(!ru_city_pixel(x,y))continue;
  if(tower_sink&&ru_tower_sink_region(x,y))continue;
  ru_set(x,y);
 }
 if(tower_sink)for(int y=0;y<RU_UTRECHT_HEIGHT;y++)for(int x=0;x<240;x++)
  if(ru_tower_sink_region(x,y)&&y+tower_sink<RU_UTRECHT_HEIGHT &&
     !ru_crack_hidden_by_houses(x,y+tower_sink))ru_set(x,y+tower_sink);
 /* Cobblestone street and opening ravine in the foreground. */
 for(int y=171;y<240;y++){
  int ry=y-171,center=119+((ry/11)%4-1)*3;
  int wid=opening?(opening*(ry+9)/75):0;
  if(wid>87)wid=87;
  for(int x=0;x<240;x++){
   int side=x<center?-1:1;
   int distance=x>center?x-center:center-x;
   if(wid&&distance<wid)continue;
   int shifted=x-side*(wid/2);
   int rowheight=(y<188?5:y<211?7:9);
   int seam=((shifted+(((ry/rowheight)&1)*9))%17)==0||ry%rowheight==0;
   if(!seam||(ru_land_hash((uint32_t)(shifted+250),(uint32_t)y)&31u)==0u)ru_set(x,y);
   if(wid&&distance>=wid&&distance<wid+3){
    if((y/6)%3!=1)ru_set(x,y);
   }
   if(wid&&distance>=wid+4&&distance<wid+12&&y>184&&((y+shifted/9)%21)<2)ru_set(x,y);
  }
 }
 if(depth){
  /* Advance the fracture upward through the city and tower. Lower houses hide
   * a section of it so the split feels like it continues behind them. */
  int head_y=171-(depth>18?(depth-18)*6:0); if(head_y<8)head_y=8;
  for(int y=head_y;y<171;y++){
   int x=ru_city_fault_x(y);
   if(depth<=18||ru_crack_hidden_by_houses(x,y))continue;
   int width=1;
   if(y<118&&depth>18)width=(depth>28&&y<110)?2:1;
   if(y>=145&&depth>10)width=2;
   for(int dx=-width;dx<=width;dx++)ru_clr(x+dx,y);
   /* Small side branches in the city and tower area. */
   if(y<118&&depth>22&&(y%11)==0){
    int dir=((y/11)&1)?1:-1;
    int bx=x+dir*(1+(depth-22)/10);
    int by=y+2;
    for(int t=0;t<4;t++)if(!ru_crack_hidden_by_houses(bx+dir*t,by+t/2))ru_clr(bx+dir*t,by+t/2);
   }
  }
  /* Zigzag fracture branches around the road opening itself. */
  for(int k=0;k<8;k++){
   int y=182+k*8;int center=119+(((y-171)/11)%4-1)*3;
   int half=(opening*(y-162))/75;
   int dir=(k&1)?1:-1;
   int x=center+dir*(half+2);
   int span=(depth*3/2)-(k*3);if(span>32)span=32;
   if(span>0){
    int x1=x+dir*span,y1=y-4+(k%3)*3;
    int dx=x1>x?x1-x:x-x1,dy=y1>y?y-y1:y1-y;
    int sx=x<x1?1:-1,sy=y<y1?1:-1,err=dx+dy;
    for(;;){
     ru_clr(x-1,y);ru_clr(x,y);ru_clr(x+1,y);
     if(x==x1&&y==y1)break;
     int e=err*2;if(e>=dy){err+=dy;x+=sx;}if(e<=dx){err+=dx;y+=sy;}
    }
   }
  }
  /* Tiny bits of tower masonry drift downward after the split opens up. */
  if(depth>18)for(int k=0;k<10;k++){
   int sx=124+(k%3);int sy=92+(k*7)%38;
   int yy=sy+tower_sink+((depth-18)*(1+(k%2)))/10;
   int xx=sx+((k&1)?1:-1)*(k%2);
   ru_set(xx,yy);if((k%3)==0)ru_set(xx,yy+1);
  }
 }
}
static const char ru_about_text[]=
 "RUPTURE is a monophonic bass synthesizer built for the music I want to make.\n\n"
 "The idea had been brewing for a long time. Last year, I built my first attempt: a Teensy 4.0 mono bass synth with mechanical keyboard keys, a custom PCB, and a 3D-printed enclosure. A hands-on project that brought the idea to life, but still left me wanting to take it further.\n\n"
 "That next step came with the M-VAVE FM1 and the amazing open-source community around it. Their work opened the door to what would become RUPTURE.\n\n"
 "At its heart, RUPTURE is personal. I built it because I wanted to play it, write music with it, and make it part of my own sound. Sharing it means that something I made for myself might find a place in your music too. And that would make it even more worthwhile.\n\n"
 "Make some noise. Make it yours. Enjoy RUPTURE.\n\n"
 "Joppe Sikken (SikkenWorks)\nUtrecht, The Netherlands\n2026";
/* Fixed 35-character wrapping, entire text kept intact. No truncated words.
 * Blank lines remain paragraph separators. SELECT scrolls one printed row. */
#define RU_ABOUT_COLS 35
static uint16_t ru_about_lines(void){
 uint16_t rows=0;const char *at=ru_about_text;
 while(*at){
  if(*at=='\n'){rows++;at++;continue;}
  uint16_t n=0;const char *start=at,*last_space=0;
  while(at[n]&&at[n]!='\n'&&n<RU_ABOUT_COLS){if(at[n]==' ')last_space=at+n;n++;}
  if(!at[n]||at[n]=='\n')at+=n;
  else if(last_space&&last_space>start)at=last_space+1;
  else at+=n;
  rows++;
 }
 return rows;
}
static uint8_t ru_about_get_line(uint16_t index,char out[RU_ABOUT_COLS+1]){
 uint16_t row=0;const char *at=ru_about_text;
 while(*at){
  if(*at=='\n'){if(row==index){out[0]=0;return 1;}row++;at++;continue;}
  uint16_t n=0;const char *start=at,*last_space=0;
  while(at[n]&&at[n]!='\n'&&n<RU_ABOUT_COLS){if(at[n]==' ')last_space=at+n;n++;}
  if(at[n]&&at[n]!='\n'&&last_space&&last_space>start)n=(uint16_t)(last_space-start);
  if(row==index){for(uint16_t i=0;i<n;i++)out[i]=at[i];out[n]=0;return 1;}
  at+=n;if(*at==' ')at++;
  row++;
 }
 out[0]=0;return 0;
}

/* Draw ONLY the 841 source modules supplied by the author: white on black.
 * The bitmap is a lossless module-by-module extraction of qr-code-original.png.
 * Neither the QR data nor its mask is ever generated from a URL in firmware.
 * Five hard 1-bit display pixels per module; 47px black quiet zone on sides. */
static void ru_bug_qr(void){
 const int x=47,y=52,scale=5;
 for(int row=0;row<29;row++)for(int col=0;col<29;col++){
  if(ru_bug_qr_white[row][col>>3]&(uint8_t)(0x80u>>(col&7))){
   ru_rect(x+col*scale,y+row*scale,scale,scale);
  }
 }
 ru_center("SCAN TO REPORT A BUG",120,211,1);
}
static void ru_render_raw(const RuState *v){ru_clear();if(v->saver_active){ru_crack_saver(v->saver_frame);return;}uint8_t page=v->page;if(page==RU_PAGE_LOADING){ru_brand(v);return;}const char *t=(page==RU_PAGE_SAFE&&v->audio_fault)?"AUDIO FAULT":(page<RU_PAGE_COUNT?ru_page_names[page]:"RUPTURE");ru_header(t,v);
 if(page==RU_PAGE_BUG_REPORT){ru_bug_qr();return;}
 if(page==RU_PAGE_MENU){
 static const char *const options[]={"FILTER","ADSR","LFO","FX","SEQUENCER","ARPEGGIATOR","SCALE GUIDE","KEYLOCK","NOTE MEMORY","PROJECTS","MIDI"};
 int sel=v->selected%11;ru_center("SELECT TO SCROLL",120,55,1);
 for(int i=-2;i<=2;i++){int j=sel+i;if(j<0||j>=11)continue;int y=114+i*25;
  if(i==0){ru_frame(9,y-6,222,22);ru_dither(13,y-3,214,3,1);}
  ru_center(options[j],120,y,1);
 }ru_footer("SEL ENTER  HOME BACK");return;}
 if(page==RU_PAGE_EQ){ru_eq_curve(v);return;}
 if(page==RU_PAGE_SEQ){ru_seq_screen(v);return;}
 if(page==RU_PAGE_FX){
  static const char *const fxnames[]={"REVERB","DELAY","CHORUS","MOSFET","COMPRESSOR","TILT EQ","OCTAVE FUZZ","SANDPAPER"};
  ru_center(fxnames[v->fx_selected%8u],120,54,1);
  ru_center(v->fx_on?"ON / SEL BYPASS":"OFF / SEL ENABLE",120,68,1);
  ru_draw_knobs(v);
  ru_footer("SELECT FX  SEL ON/OFF");return;
 }
 if(page==RU_PAGE_SUB1||page==RU_PAGE_SUB2){
  ru_frame(8,50,224,30);
  ru_center(page==RU_PAGE_SUB1?"SUB OSCILLATOR 1":"SUB OSCILLATOR 2",120,60,1);
  ru_draw_knobs(v);
  /* Fifth visual knob follows the physical ALGORITHM encoder. */
  ru_knob(120,135,v->sub_blend);
  ru_center("BLEND",120,160,1);
  char blend[12];ru_uint(v->sub_blend,blend);ru_center(blend,120,171,1);
  ru_footer("ALGORITHM BLEND  HOME NEXT");return;
 }
 if(page==RU_PAGE_SHAPE){ru_frame(8,50,224,30);ru_center("SOUND SHAPING",120,61,1);ru_draw_knobs(v);ru_footer("ALG OSC / HOME SUB 1");return;}
 if(page==RU_PAGE_PLAY){
  /* Copy the user's ORIGINAL 1-bit PNG frame directly into the existing
   * 240-wide 1-bit UI canvas. X=8 is byte aligned (one byte offset).
   * No scaling, line drawing, buffers, compositing, alpha or audio work.
   * The outer border is INCLUDED in every source sprite. */
  uint8_t frame=v->dam_frame<RU_DAM_NUM_FRAMES?v->dam_frame:0u;
  for(uint32_t y=0u;y<RU_DAM_H;y++){
   const uint8_t *p=ru_dam_sprites[frame][y];
   uint32_t dst=(RU_DAM_Y+y)*(RU_W/8u)+(RU_DAM_X/8u);
   for(uint32_t b=0u;b<RU_DAM_ROW_BYTES;b++)ru_bits[dst+b]=p[b];
  }
  ru_draw_knobs(v);ru_footer("ALG OSC / HOME SHAPE");return;
 }
 if(page==RU_PAGE_FOREIGN||page==RU_PAGE_FORMAT_CONFIRM){ru_center("FOREIGN DATA DETECTED",120,63,1);ru_center("FORMAT USER DATA?",120,91,1);ru_frame(18,133,98,42);ru_frame(124,133,98,42);ru_center("NO",66,150,2);ru_center("YES",173,150,2);ru_dither(v->format_yes?128:22,137,90,4,1);ru_footer("SEL CONFIRM   HOME BACK");return;}
 if(page==RU_PAGE_SAFE){
  /* A DMA deadline shutdown is not a boot/storage Safe Mode. Keep the
   * protective audio stop but show the actual reason to the musician. */
  if(v->audio_fault){
   ru_center("AUDIO STOP",120,79,2);
   ru_center("DMA TIMING FAULT",120,116,1);
   ru_center("HOME RESTART",120,155,1);
   ru_footer("GLO AUDIO DIAGNOSTICS");
  }else{
   ru_center("SAFE MODE",120,84,2);
   ru_center("USB RECOVERY",120,119,1);
   ru_center("BOOT DATA PROTECTED",120,153,1);
   ru_footer("HOME BACK");
  }
  return;
 }
 if(page==RU_PAGE_UPDATE){
  ru_center("FIRMWARE UPDATE",120,87,1);
  const char *phase=v->selected==3u?"STAGING OTA LOADER":v->selected==4u?"READY FOR HANDOFF":
   v->selected==5u?"RESTARTING LOADER":v->selected==9u?"OTA PREP FAILED":"VERIFYING PACKAGE";
  ru_center(phase,120,105,1);
  ru_frame(17,120,207,22);
  ru_dither(20,123,(v->k[0]*2u>200u?200u:v->k[0]*2u),15,1);
  char number[12];ru_uint(v->k[0],number);ru_center(number,120,159,1);
  if(v->selected==9u){
   char code[16]="ERROR ";ru_uint(v->ota_error,code+6);
   ru_center(code,120,183,1);
   ru_footer("OTA ABORTED / NO WRITE");
  }else{
   ru_center("FLASH PROGRESS IN WEBINSTALLER",120,180,1);
   ru_footer("DO NOT DISCONNECT");
  }
  return;
 }
 if(page==RU_PAGE_HELP){
  uint8_t pageindex=v->help_page%RU_HELP_CARD_COUNT;
  const struct ru_help_card *card=&ru_help_cards[pageindex];
  char number[12],total[12];ru_uint(pageindex+1u,number);ru_uint(RU_HELP_CARD_COUNT,total);
  ru_frame(8,49,224,152);
  ru_dither(11,52,218,4,1);
  ru_text(card->title,16,67,1);
  ru_text(number,180,67,1);ru_text("/",194,67,1);ru_text(total,205,67,1);
  ru_line(15,84,224,84);
  for(uint8_t i=0u;i<3u;i++){
   uint8_t n=(uint8_t)(v->help_scroll+i);
   if(n>=5u)break;
   int y=95+(int)i*30;
   ru_frame(15,y-3,206,26);
   ru_rect(19,y,3,18);
   ru_text(card->line[n],29,y+4,1);
  }
  for(uint8_t i=0u;i<3u;i++)ru_rect(106+(int)i*11,192,(v->help_scroll==i)?8:4,3);
  ru_footer("SELECT CARDS  ALG SCROLL");return;
 }
 if(page==RU_PAGE_DIAG){
  const char *names[]={"WALL MAX","DSP MAX","RENDER","PUBLISH","MIN SLACK","LATE","GUARD","SCOPE SKIP"};
  uint32_t vals[]={v->diag_wall,v->diag_dsp,v->diag_render,v->diag_copy,
   v->diag_slack,v->diag_late,v->diag_guard,v->diag_scope};
  ru_center("READ ONLY / LIVE DMA",120,53,1);
  for(int i=0;i<8;i++){
   int y=77+i*17;char value[12];ru_uint(vals[i],value);
   ru_text(names[i],10,y,1);ru_text(value,151,y,1);
   if(i<5)ru_text("%",211,y,1);
  }
  ru_footer("HOME BACK / LATE AND GUARD 0");return;
 }
 if(page==RU_PAGE_ABOUT){
  char line[RU_ABOUT_COLS+1];uint16_t count=ru_about_lines();
  ru_text("THE STORY OF RUPTURE",12,53,1);
  ru_line(12,67,228,67);
  for(uint8_t i=0u;i<11u;i++)if(ru_about_get_line((uint16_t)v->about_scroll+i,line))ru_text(line,12,75+(int)i*13,1);
  ru_frame(231,75,4,143);
  uint16_t usable=count>11u?count-11u:1u;
  int thumb=(int)v->about_scroll*120/(int)usable;
  ru_rect(231,76+thumb,4,15);
  ru_footer("SELECT SCROLL  HOME BACK");return;
 }
 if(page==RU_PAGE_OTHER){
  ru_frame(12,76,216,66);ru_dither(15,81,210,4,1);
  ru_rect(17,90,3,33);ru_text("SCREENSAVER",30,96,1);
  ru_text(">",208,99,1);ru_footer("SEL OPEN  HOME BACK");return;
 }
 if(page==RU_PAGE_SAVER_OPTIONS){
  const char *rows[]={"SCREENSAVER","START AFTER"};
  for(int i=0;i<2;i++){
   int y=73+i*66;ru_frame(12,y,216,58);
   if(v->saver_options_selected==(uint8_t)i){ru_dither(15,y+3,210,4,1);ru_rect(17,y+14,3,32);}
   ru_text(rows[i],29,y+12,1);
   if(i==0){ru_frame(40,y+31,12,12);if(v->saver_enabled){ru_line(42,y+37,45,y+41);ru_line(45,y+41,50,y+33);}
    ru_text(v->saver_enabled?"ON":"OFF",71,y+33,1);
   }else{
    char seconds[12];ru_uint(ru_saver_time_values[v->saver_delay_index%8u],seconds);
    ru_text(seconds,71,y+33,1);ru_text("SECONDS",108,y+33,1);
   }
   if(v->saver_options_selected==(uint8_t)i)ru_text(">",208,y+21,1);
  }
  ru_footer("SELECT ROW  SEL ON/OFF  K1 TIME");return;
 }
 if(page==RU_PAGE_STORAGE){
  static const char *const storage[]={"PROJECTS","WIPE ALL"};
  for(int i=0;i<2;i++){
   int y=73+i*68;ru_frame(12,y,216,59);
   if(v->selected==(uint8_t)i){ru_dither(15,y+3,210,4,1);ru_rect(17,y+14,3,31);}
   ru_text(storage[i],29,y+15,1);
   if(v->selected==(uint8_t)i)ru_text(">",207,y+22,1);
  }
  ru_footer("SELECT CARD  SEL OPEN");return;
 }
 if(page==RU_PAGE_STORAGE_PROJECTS || page==RU_PAGE_STORAGE_SEQUENCES){
  int is_seq=page==RU_PAGE_STORAGE_SEQUENCES;
  int total=is_seq?8:12,selected=is_seq?v->manager_seq:v->manager_project;
  int first=selected>2?selected-2:0;if(first>total-6)first=total-6;if(first<0)first=0;
  ru_center(is_seq?"PROJECT SEQUENCES":"STORED PROJECTS",120,54,1);
  for(int i=0;i<6;i++){
   int slot=first+i,y=73+i*23;
   uint8_t occupied=is_seq?!!(v->manager_seq_mask&(1u<<slot)):!!(v->project_mask&(1u<<slot));
   const char *name=is_seq?v->manager_seq_names[slot]:v->project_names[slot];
   char num[12];ru_uint((uint32_t)slot+1u,num);
   if(selected==slot){ru_frame(9,y-3,221,20);ru_dither(12,y,214,2,1);}
   ru_text(num,16,y+3,1);ru_text(occupied?(name[0]?name:"UNTITLED"):"EMPTY",48,y+3,1);
  }
  if(v->manager_status)ru_center(v->manager_status==1?"SAVED":v->manager_status==2?"NO EMPTY SLOT":v->manager_status==3?"ACTION FAILED":v->manager_status==5?"SHARED WAVES / LOCK":"CONFIRM ACTION",120,214,1);
  ru_footer("SELECT SLOT  SEL ACTIONS");return;
 }
 if(page==RU_PAGE_STORAGE_ACTIONS || page==RU_PAGE_STORAGE_SEQ_ACTIONS){
  int sq=page==RU_PAGE_STORAGE_SEQ_ACTIONS;
  static const char *const pacts[]={"OPEN PROJECT","SEQUENCES","DUPLICATE","RENAME","DELETE"};
  static const char *const sacts[]={"OPEN SEQUENCE","DUPLICATE","RENAME","DELETE"};
  int count=sq?4:5;
  ru_center(sq?"SEQUENCE ACTIONS":"PROJECT ACTIONS",120,53,1);
  for(int i=0;i<count;i++){
   int y=75+i*28;ru_frame(13,y,214,24);
   if(v->manager_action==(uint8_t)i){ru_rect(17,y+3,3,18);ru_dither(23,y+3,198,3,1);}
   ru_text(sq?sacts[i]:pacts[i],30,y+8,1);
  }
  if(v->manager_status){
    ru_center(v->manager_status==1u?"SAVED":v->manager_status==2u?"NO EMPTY SLOT":
     v->manager_status==4u?"SEL AGAIN TO CONFIRM":v->manager_status==5u?"SHARED WAVES / LOCK":"ACTION FAILED",120,215,1);
  }
  ru_footer("SELECT ACTION  SEL RUN");return;
 }
 if(page==RU_PAGE_WIPE_ALL){
  ru_center("WARNING: WIPE ALL",120,58,1);
  ru_center("ERASE ALL STORED DATA",120,83,1);
  ru_center("PROJECTS AND SEQUENCES",120,100,1);
  ru_center("INCLUDING OLD FIRMWARE",120,117,1);
  ru_center("THIS CANNOT BE UNDONE!",120,138,1);
  ru_frame(14,157,100,43);ru_frame(126,157,100,43);
  if(v->manager_confirm==1u)ru_dither(130,160,92,5,1);else ru_dither(18,160,92,5,1);
  ru_center("NO",64,174,2);ru_center("YES",176,174,2);
  ru_footer("SELECT NO/YES  SEL CONFIRM");return;
 }
 if(page==RU_PAGE_GLO){
  static const char *const cards[]={"SETTINGS","HELP","AUDIO DIAGNOSTICS","ABOUT","REPORT A BUG"};
  for(int i=0;i<5;i++){
   int y=48+i*34;ru_frame(12,y,216,30);
   if(v->selected==(uint8_t)i){ru_dither(15,y+3,210,3,1);ru_rect(16,y+8,3,14);}
   ru_text(cards[i],28,y+11,1);
   if(v->selected==(uint8_t)i)ru_text(">",207,y+11,1);
  }
  ru_footer("SELECT CARD  SEL OPEN");return;}
 if(page==RU_PAGE_PROJECTS||page==RU_PAGE_SEQ_SLOTS){
  int total=page==RU_PAGE_PROJECTS?12:8;
  int selected=(page==RU_PAGE_PROJECTS?v->project:v->seq_slot)-1;
  if(selected<0)selected=0;
  if(selected>=total)selected=total-1;
  ru_center(page==RU_PAGE_PROJECTS?"PROJECT LIBRARY":"SEQUENCE LIBRARY",120,52,1);
  int first=selected>=2?selected-2:0;
  if(first>total-5)first=total-5;
  if(first<0)first=0;
  for(int n=0;n<5;n++){
   int slot=first+n;
   int y=78+n*24;
   uint8_t present=page==RU_PAGE_PROJECTS?
      !!(v->project_mask&(1u<<slot)):!!(v->seq_mask&(1u<<slot));
   char number[8];ru_uint((uint32_t)(slot+1),number);
   if(slot==selected){ru_frame(8,y-5,224,22);ru_dither(11,y-2,216,2,0);}
   ru_text(number,16,y,1);
   const char *label=page==RU_PAGE_PROJECTS?v->project_names[slot]:v->seq_names[slot];
   ru_text(present&&label[0]?label:"EMPTY",49,y,1);
   /* Browser cursor (box) and loaded identity (ACTIVE) are independent.
    * Preserve the marker while browsing other occupied slots. */
   uint8_t active=(uint8_t)(page==RU_PAGE_PROJECTS?
      v->active_project==(uint8_t)(slot+1):v->active_seq_slot==(uint8_t)(slot+1));
   if(active && present)ru_text("ACTIVE",175,y,1);
  }
  ru_center("SEL OPEN  /  SAVE STORE",120,203,1);
  uint8_t status=page==RU_PAGE_PROJECTS?v->project_status:v->seq_status;
  if(status){
   const char *message=status==1u?"LOADED":status==2u?"SAVED":status==4u?"SAVE AGAIN TO REPLACE":status==5u?"SEL AGAIN TO OPEN":
    v->store_error==-5?"STORAGE FULL":v->store_error==-3?"FLASH I/O ERROR":
    v->store_error==-4?"VERIFY FAILED":"SAVE FAILED";
   ru_center(message,120,216,1);
  }
  ru_footer("EMPTY: SEL OR SAVE TO NAME");return;
 }
 if(page==RU_PAGE_NAME){
  ru_center(v->edit_is_sequence?"NAME SEQUENCE":"NAME PROJECT",120,57,1);
  ru_frame(9,95,222,38);ru_text(v->edit_name,12,107,1);
  ru_frame(11+(int)v->edit_cursor*6,120,6,3);
  ru_center("SELECT POSITION",120,155,1);
  ru_center("ALGORITHM CHARACTER",120,176,1);
  if((v->edit_is_sequence?v->seq_status:v->project_status)==3u){
   const char *problem=v->store_error==-5?"STORAGE FULL":
    v->store_error==-3?"FLASH I/O ERROR":
    v->store_error==-4?"VERIFY FAILED":
    v->store_error==-2?"STORAGE LOCKED":
    v->store_error==-6?"INCOMPATIBLE DATA":"SAVE BLOCKED / RETRY";
   ru_center(problem,120,198,1);
  }
  ru_footer("SAVE TO STORE");return;
 }
 if(page==RU_PAGE_GUIDE||page==RU_PAGE_KEYLOCK){ru_center("ROOT / SCALE / LOCK",120,57,1);ru_draw_knobs(v);ru_footer("K1 KEY  K2 SCALE  K3 LOCK");return;}
 if(page==RU_PAGE_NOTE_MEMORY){ru_center("6 NOTE MEMORY / MONO",120,57,1);ru_draw_knobs(v);ru_footer("6 NOTES / PRIORITY / GLIDE");return;}
 if(page==RU_PAGE_AUTOLOAD){
  ru_center("AUTO LOAD PROJECT",120,46,1);
  ru_center("SELECT PROJECT AT BOOT",120,61,1);
  int first=0;
  if(v->selected>3u)first=(int)v->selected-3;
  if(first>7)first=7;
  for(int i=0;i<5;i++){
   int choice=first+i;
   if(choice>12)break;
   uint8_t present=(uint8_t)(choice==0||(v->project_mask&(1u<<(choice-1))));
   if(!present)continue;
   int y=77+i*29;
   ru_frame(13,y,214,27);
   if(choice==(int)v->selected)ru_rect(17,y+7,3,13);
   if(choice==0)ru_text("NEW",28,y+9,1);
   else{
    char name[14];for(int k=0;k<12;k++)name[k]=v->project_names[choice-1][k];name[12]=0;
    if(name[0])ru_text(name,28,y+9,1);
    else{char label[12]="PROJECT 00";label[8]=(char)('0'+choice/10);label[9]=(char)('0'+choice%10);ru_text(label,28,y+9,1);}
   }
   if(choice==(int)v->boot_choice)ru_text("*",204,y+9,1);
  }
  ru_footer("SELECT CHOOSE / SEL SAVE");return;
 }
 if(page==RU_PAGE_SETTINGS){
  if(v->settings_open==0u){
   static const char *const names[]={"MASTER VOLUME","AUTO SAVE","AUTO LOAD PROJECT","STORAGE","OTHER","CRT / VCR LOOK"};
   for(int i=0;i<6;i++){
    int y=52+i*28;ru_frame(12,y,216,26);
    if(v->selected==(uint8_t)i){ru_dither(15,y+3,210,3,1);ru_rect(17,y+8,3,12);}
    ru_text(names[i],27,y+9,1);
    if(i==0){char num[12];ru_uint(v->master,num);int l=ru_strlen(num);if(l<9){num[l]='%';num[l+1]=0;}ru_center(num,195,y+9,1);}
    if(i==1){ru_center(v->auto_save?"ON":"OFF",195,y+9,1);}
    if(i==2){if(v->boot_choice==0)ru_center("NEW",199,y+9,1);else{char n[5];ru_uint(v->boot_choice,n);ru_center(n,199,y+9,1);}}
    if(i==5)ru_center(v->crt_enabled?"ON":"OFF",195,y+9,1);
    if(v->selected==(uint8_t)i)ru_text(">",217,y+9,1);
   }
   ru_footer(v->selected==0u?"K1 VOLUME / SEL OPEN":v->selected==1u?"SEL TOGGLE AUTO SAVE":v->selected==5u?"SEL TOGGLE CRT / VCR":"SELECT / SEL OPEN");return;
  }
  ru_frame(12,72,216,111);ru_dither(15,75,210,4,1);
  char num[12];ru_uint(v->master,num);ru_center("MASTER VOLUME",120,89,1);
  ru_center(num,120,112,2);ru_center("PERCENT / MAX 200",120,150,1);
  ru_footer("SELECT OR K1 ADJUST / SEL BACK");return;
 }
 if(page==RU_PAGE_FILTER){ru_center("24DB LADDER + BANDPASS",120,57,1);ru_draw_knobs(v);ru_footer("K1 FILTER TYPE");return;}
 if(page==RU_PAGE_MIDI){ru_center("USB + TRS MIDI IN",120,57,1);ru_draw_knobs(v);ru_footer("USB/TRS MIDI INPUT");return;}
 ru_draw_knobs(v);ru_footer("HOME BACK / HOLD MAIN");
}

/* V1.52: keep the original 1-bit source pixel-identical. CRT optics are
 * applied only during RGB565 LCD transfer, preserving every text stroke.
 * Report-a-bug QR bypasses both lens warp and scanline dimming. */
static void ru_render(const RuState *v){
 ru_render_raw(v);
}
