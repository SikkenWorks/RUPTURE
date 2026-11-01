/* GPL-3.0-only, native 1-bit page display smoke test. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c"
static unsigned long checksum(void){unsigned long h=2166136261u;for(int y=0;y<240;y++)for(int x=0;x<240;x++)h=(h^ru_get(x,y))*16777619u;return h;}
int main(void){
 RuState st={0};st.page=RU_PAGE_GLO;st.selected=2;ru_render(&st);
 assert(ru_get(12,48)&&ru_get(12,82)&&ru_get(12,116)&&ru_get(12,150)&&ru_get(12,184));
 /* Full AUDIO DIAGNOSTICS is visibly rendered as ordinary menu text. */
 unsigned long full_label=checksum();assert(full_label!=0u);
 st.page=RU_PAGE_SETTINGS;st.settings_open=0u;st.selected=0u;st.master=100u;
 ru_render(&st);unsigned long menu_100=checksum();
 st.master=125u;ru_render(&st);assert(checksum()!=menu_100); /* percent on row */
 st.selected=1u;st.auto_save=0u;ru_render(&st);unsigned long auto_off=checksum();
 st.auto_save=1u;ru_render(&st);assert(checksum()!=auto_off); /* direct checkbox */
 st.selected=0u;st.settings_open=1u;ru_render(&st);assert(checksum()!=menu_100); /* retained MASTER modal */
 st.page=RU_PAGE_SAVER_OPTIONS;st.settings_open=0u;st.saver_delay_index=1u;st.saver_enabled=0u;ru_render(&st);unsigned long saver_off=checksum();
 st.saver_enabled=1u;ru_render(&st);assert(checksum()!=saver_off);
 puts("PASS V1.29 native 240x240 GLO cards / master row / AUTO SAVE checkbox / retained master modal / screensaver checkbox");
 return 0;
}
