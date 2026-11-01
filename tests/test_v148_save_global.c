/* V1.48: comprehensive routing audit for every native screen. */
#include <assert.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_ui.c"
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_save_route.h"
int main(void){
 const unsigned exceptional[]={RU_PAGE_NAME,RU_PAGE_UPDATE,RU_PAGE_FOREIGN,
  RU_PAGE_FORMAT_CONFIRM,RU_PAGE_SAFE,RU_PAGE_LOADING,RU_PAGE_TRANSIENT,RU_PAGE_FOLDER};
 int projects=0,sequences=0,no=0;
 for(unsigned p=0;p<RU_PAGE_COUNT;p++){
  uint8_t dest=ru_save_destination(p);
  if(p==RU_PAGE_SEQ||p==RU_PAGE_SEQ_SLOTS){assert(dest==RU_SAVE_SEQUENCE_MENU);sequences++;}
  else{
   int exempt=0;for(unsigned j=0;j<sizeof(exceptional)/sizeof(exceptional[0]);j++)if(exceptional[j]==p)exempt=1;
   if(exempt){assert(dest==RU_SAVE_NO_MENU);no++;}
   else{assert(dest==RU_SAVE_PROJECT_MENU);projects++;}
  }
 }
 assert(ru_save_destination(RU_PAGE_COUNT)==RU_SAVE_NO_MENU);
 assert(ru_save_destination(255)==RU_SAVE_NO_MENU);
 assert(projects==39 && sequences==2 && no==8);
 assert(ru_save_pressed(1u<<9,1u<<9));
 assert(!ru_save_pressed(1u<<9,(1u<<9)|(1u<<2)));
 assert(!ru_save_pressed(0u,0u));
 printf("PASS: 49 screen IDs: %d project, %d sequence, %d protected/edit exceptions\n",projects,sequences,no);
 return 0;
}
