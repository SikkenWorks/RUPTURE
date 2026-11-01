/* Pixels of ACTIVE on a row must follow the opened slot, not the cursor. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c"
static int marker(void){int n=0;for(int y=78;y<86;y++)for(int x=175;x<213;x++)n+=ru_get(x,y);return n;}
int main(void){
 RuState v={0};v.page=RU_PAGE_PROJECTS;v.project=2;v.project_mask=3;
 memcpy(v.project_names[0],"FIRST",5);memcpy(v.project_names[1],"SECOND",6);
 ru_render(&v);int before=marker();
 v.active_project=1;ru_render(&v);int after=marker();
 assert(after>before);
 v.page=RU_PAGE_SEQ_SLOTS;v.seq_slot=2;v.seq_mask=3;
 memcpy(v.seq_names[0],"INTRO",5);memcpy(v.seq_names[1],"VERSE",5);
 v.active_project=0;v.active_seq_slot=0;ru_render(&v);before=marker();
 v.active_seq_slot=1;ru_render(&v);after=marker();
 assert(after>before);
 puts("PASS V1.33 native 240x240 ACTIVE markers independent of cursor in both browsers");
 return 0;
}
