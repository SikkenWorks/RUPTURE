#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_glide_tunnel_playback.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/native_music_labels.h"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c"

static void test_motion(void){
 uint8_t frame=0u,tick=0u;
 uint32_t last_ms=1000u;
 assert(!ru_glide_tunnel_play_step(1000u,0u,&frame,&tick,&last_ms,12u));
 assert(!ru_glide_tunnel_play_step(1060u,1u,&frame,&tick,&last_ms,12u));
 assert(ru_glide_tunnel_play_step(1085u,1u,&frame,&tick,&last_ms,12u));
 assert(frame==0u&&tick==1u);
 assert(!ru_glide_tunnel_play_step(1100u,1u,&frame,&tick,&last_ms,12u));
 assert(ru_glide_tunnel_play_step(1170u,1u,&frame,&tick,&last_ms,12u));
 assert(frame==0u&&tick==2u);
 assert(ru_glide_tunnel_play_step(1255u,1u,&frame,&tick,&last_ms,12u));
 assert(frame==1u&&tick==3u);
 /* Releasing the last sounding voice stops both walking and camera jitter. */
 assert(ru_glide_tunnel_play_step(1260u,0u,&frame,&tick,&last_ms,12u));
 assert(frame==0u&&tick==0u&&last_ms==1260u);
 assert(!ru_glide_tunnel_play_step(1275u,0u,&frame,&tick,&last_ms,12u));
 assert(!ru_glide_tunnel_play_step(1330u,1u,&frame,&tick,&last_ms,12u));
 assert(frame==0u&&tick==0u);
 assert(ru_glide_tunnel_play_step(1360u,1u,&frame,&tick,&last_ms,12u));
 assert(frame==0u&&tick==1u);
 /* The existing 24-tick shake loop and 12-frame sprite loop remain intact. */
 frame=0u;tick=0u;last_ms=0u;
 for(uint32_t i=1u;i<=36u;i++)
  assert(ru_glide_tunnel_play_step(i*85u,1u,&frame,&tick,&last_ms,12u));
 assert(frame==0u&&tick==12u);
 /* Unsigned millisecond-wrap arithmetic remains safe. */
 frame=0u;tick=0u;last_ms=0xfffffff0u;
 assert(ru_glide_tunnel_play_step(0x45u,1u,&frame,&tick,&last_ms,12u));
 assert(tick==1u&&frame==0u);
}

static void test_guide_labels(void){
 static const uint8_t modes[]={0u,1u,3u,2u};
 static const char *const labels[]={"OFF","GUIDE","STRICT","LOCK"};
 RuState v={0};v.page=RU_PAGE_GUIDE;
 static const uint8_t knob_positions[]={0u,33u,67u,100u};
 for(uint8_t i=0u;i<4u;i++){
  v.keylock=modes[i];v.k[2]=modes[i];
  assert(ru_knob_normal(&v,2)==knob_positions[i]);
  ru_clear();ru_draw_knobs(&v);
  uint8_t drawn[128*8];
  for(int y=217;y<225;y++)for(int x=0;x<128;x++)
   drawn[(y-217)*128+x]=ru_get(x,y);
  ru_clear();ru_center(labels[i],64,217,1);
  for(int y=217;y<225;y++)for(int x=0;x<128;x++)
   assert(drawn[(y-217)*128+x]==ru_get(x,y));
  if(i<3u)assert(etch_lock_step(modes[i],1)==modes[i+1u]);
 }
 assert(etch_lock_step(2u,1)==2u);
 assert(etch_lock_step(0u,-1)==0u);
}

int main(void){
 test_motion();test_guide_labels();
 puts("PASS V1.99 GLIDE camera gated by sounding notes; GUIDE text OFF/GUIDE/STRICT/LOCK");
 return 0;
}
