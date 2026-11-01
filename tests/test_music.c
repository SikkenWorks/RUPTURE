#include "../src/rupture_music.h"
#include <assert.h>
#include <stdio.h>
int main(void){
 assert(rupture_in_scale(60,0,RU_SCALE_MAJOR));assert(rupture_in_scale(64,0,RU_SCALE_MAJOR));assert(!rupture_in_scale(61,0,RU_SCALE_MAJOR));
 assert(rupture_keylock(61,0,RU_SCALE_MAJOR)==60);
 for(int key=0;key<128;key++)for(int root=0;root<12;root++)for(int scale=0;scale<6;scale++)assert(rupture_in_scale(rupture_keylock(key,root,scale),root,scale));
 uint8_t keys[]={48,55,52};RuptureArp a={0};a.octaves=2;a.mode=RU_ARP_UP;
 assert(rupture_arp_next(&a,keys,3)==48);assert(rupture_arp_next(&a,keys,3)==52);assert(rupture_arp_next(&a,keys,3)==55);assert(rupture_arp_next(&a,keys,3)==60);
 a.index=0;a.mode=RU_ARP_DOWN;assert(rupture_arp_next(&a,keys,3)==67);
 a.index=0;a.mode=RU_ARP_RANDOM;for(int j=0;j<500;j++){uint8_t x=rupture_arp_next(&a,keys,3);assert(x<128);}
 puts("PASS: scale guide, Keylock full MIDI range, 5 arpeggiator modes");return 0;
}
