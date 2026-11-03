/* RUPTURE V1.65: pixel-exact source bitmap comparison under 1-bit UI shaking.
 * Earlier V1.56/1.57 tests compared to unshifted sprites and necessarily fail
 * once V1.58 added intentional whole-panel shake. This verifies true mapping. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_eq2.c"
static int bit(const uint8_t src[RU_PANEL_HEIGHT][RU_PANEL_BYTES],int x,int y){
 if(x<0||y<0||x>=(int)RU_PANEL_BYTES*8||y>=(int)RU_PANEL_HEIGHT)return 0;
 return (src[y][x>>3]>>(7-(x&7)))&1;
}
static uint32_t outside_hash(void){
 uint32_t h=2166136261u;
 for(int y=0;y<RU_H;y++){
  if(y>=RU_PANEL_Y && y<RU_PANEL_Y+RU_PANEL_HEIGHT)continue;
  for(int x=0;x<RU_W;x++)h=(h^(uint32_t)ru_get(x,y))*16777619u;
 }
 return h;
}
static void check(int page,int frame,int shake,uint32_t *expected_outside){
 RuState v={0};v.page=(uint8_t)page;v.panel_frame=(uint8_t)frame;v.panel_shake=(uint8_t)shake;
 const uint8_t (*a)[RU_PANEL_BYTES] = page==RU_PAGE_LFO?ru_lfo_sprites[frame]:
  page==RU_PAGE_ENV?ru_env_sprites[frame]:ru_volcano_sprites[frame];
 int dx=0,dy=0;
 ru_panel_shake((uint8_t)page,(uint8_t)frame,(uint8_t)shake,&dx,&dy);
 ru_render(&v);
 uint32_t out=outside_hash();
 if(!*expected_outside)*expected_outside=out;
 else assert(out==*expected_outside);
 for(int y=RU_PANEL_Y;y<(int)(RU_PANEL_Y+RU_PANEL_HEIGHT);y++)
  for(int x=0;x<RU_W;x++){
   int want=bit(a,x-(int)RU_PANEL_X-dx,y-(int)RU_PANEL_Y-dy);
   if(ru_get(x,y)!=want){
    fprintf(stderr,"sprite mismatch page=%d frame=%d shake=%d x=%d y=%d\n",page,frame,shake,x,y);
    assert(ru_get(x,y)==want);
   }
  }
}
int main(void){
 for(int page=RU_PAGE_ENV;page<=RU_PAGE_LFO;page++){
  uint32_t outside=0;
  for(int frame=0;frame<20;frame++)for(int shake=0;shake<3;shake++)check(page,frame,shake,&outside);
 }
 uint32_t outside=0;
 for(int frame=0;frame<20;frame++)for(int shake=0;shake<3;shake++)check(RU_PAGE_SHAPE,frame,shake,&outside);
 puts("PASS V1.65: 180 UI-state sprite render mappings; original 1-bit PNG art retained with expected shake");
 return 0;
}
