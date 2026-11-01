#include "../src/rupture_first_boot.h"
#include "../src/rupture_data.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
int main(void){uint8_t image[32];for(int i=0;i<32;i++)image[i]=255;
 assert(rupture_classify_header(image,32)==RUPTURE_STORAGE_EMPTY);
 image[0]='E';assert(rupture_classify_header(image,32)==RUPTURE_STORAGE_FOREIGN);
 image[0]='T';image[1]='P';image[2]='U';image[3]='R';image[4]=1;image[5]=image[6]=image[7]=0;
 assert(rupture_classify_header(image,32)==RUPTURE_STORAGE_FOREIGN);
 RuptureLibrary *lib=malloc(sizeof(*lib));assert(lib);rupture_library_init(lib);
 assert(rupture_classify_header((const uint8_t*)lib,sizeof(*lib))==RUPTURE_STORAGE_OWNED);
 lib->project[0].root=5;assert(rupture_classify_header((const uint8_t*)lib,sizeof(*lib))==RUPTURE_STORAGE_FOREIGN);free(lib);
 assert(!rupture_format_authorized(0,1,1,0));assert(!rupture_format_authorized(1,0,1,0));
 assert(!rupture_format_authorized(1,1,0,0));assert(!rupture_format_authorized(1,1,1,1));assert(rupture_format_authorized(1,1,1,0));
 puts("PASS: empty / own / foreign read-only detection, dual-confirmation gates");return 0;}
