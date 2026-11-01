/* Regression uses the real NOR emulator and sequence listing API. */
#define main rupture_v128_test_main
#include "test_v128_storage.c"
#undef main
int main(void){
 memset(flashbuf,255,sizeof(flashbuf));
 make_project(0);
 for(unsigned i=24;i<27;i++)l.patches[i].valid=1u;
 memcpy(l.patches[24].name,"ETCH 24",7);
 memcpy(l.patches[25].name,"CUSTOM",6);
 memcpy(l.patches[26].name,"ETCH 26",7);
 etch_pattern_clear(&p);
 p.steps[0].count=1;p.steps[0].notes[0].note=36;
 p.steps[0].notes[0].velocity=100;p.steps[0].notes[0].length=1;
 assert(etch_pattern_bank_capture(&cfg.patterns,26,&p)==0);
 etch_store_project_select(0u);
 etch_store_project_name("TEST PROJECT");
 assert(etch_store_save_project(&io,&l,&p,&cfg)==ETCH_STORE_OK);
 etch_store_project_select(1u); /* browsing another project */
 uint8_t mask=0;char names[8][13];
 assert(etch_store_project_sequence_list(&io,0u,&mask,names)==ETCH_STORE_OK);
 assert(!(mask&1u) && names[0][0]==0); /* old ETCH empty placeholder */
 assert((mask&2u) && names[1][0]=='C'); /* valid deliberate empty sequence */
 assert((mask&4u) && names[2][0]=='E'); /* real pattern, even if named ETCH 26 */
 puts("PASS V1.30 project-independent sequence listing: legacy empty filtered, custom empty + real pattern retained");
 return 0;
}
