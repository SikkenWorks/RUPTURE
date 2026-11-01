#include <assert.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c"
int main(void){
 ru_clear();ru_battery(100,0);
 for(int y=9;y<19;y++)for(int x=204;x<228;x++)assert(ru_get(x,y));
 ru_clear();ru_battery(75,0);assert(!ru_get(227,11));
 ru_clear();ru_battery(-1,0);assert(ru_get(206,12));
 RuState state={0};state.page=RU_PAGE_PROJECTS;state.project=1;
 state.project_mask=1;state.project_status=5;state.show_battery=1;state.battery=100;
 ru_render(&state);assert(ru_get(227,11));
 state.page=RU_PAGE_NAME;state.edit_name[0]='R';state.edit_name[1]='I';state.edit_name[2]='F';state.edit_name[3]='F';
 ru_render(&state);assert(ru_get(11,95));
 puts("PASS V1.26: full charge spans entire battery interior, 75% remains partial, menu and name editor draw");
 return 0;
}
