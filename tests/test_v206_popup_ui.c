#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c"

static uint32_t middle_hash(void){
 uint32_t h=2166136261u;
 for(int y=76;y<165;y++)for(int x=13;x<227;x++)h=(h^ru_get(x,y))*16777619u;
 return h;
}
int main(void){
 RuState v={0};
 v.page=RU_PAGE_SEQ;
 v.seq_length=16u;
 v.seq_current_step=3u;
 v.seq_first_note=53u;
 v.seq_note_count=1u;
 v.k[0]=16u;v.k[1]=120u;v.k[2]=98u;v.k[3]=70u;
 ru_render(&v);
 uint32_t before=middle_hash();
 v.preset_assign=RU_PRESET_ASSIGN_DEFAULT;
 v.preset_popup_visible=1u;
 v.preset_popup_value[0]='1';v.preset_popup_value[1]='2';
 v.preset_popup_value[2]='0';v.preset_popup_value[3]=0;
 ru_render(&v);
 uint32_t bpm=middle_hash();
 assert(before!=bpm && ru_get(13,76) && ru_get(226,164));
 v.preset_assign=36u; /* SCALE GUIDE ROOT */
 v.preset_popup_value[0]='C';v.preset_popup_value[1]='#';
 v.preset_popup_value[2]=0;
 ru_render(&v);
 uint32_t scale=middle_hash();
 assert(scale!=bpm && ru_get(13,76) && ru_get(226,164));
 /* Storage feedback overrides the knob popup without changing the data. */
 v.preset_save_stage=2u;v.preset_save_progress=55u;
 ru_render(&v);
 assert(middle_hash()!=scale);
 puts("PASS V2.0.6 PRESETS popup redraw on populated SEQ, title/value, storage priority");
 return 0;
}
