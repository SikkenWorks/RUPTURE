#include <stdio.h>
#include <assert.h>
static int level(int v){if(v<100||v>950)return -1;return v>=578?100:v>=558?75:v>=531?50:25;}
int main(void){assert(level(-1)==-1&&level(0)==-1&&level(531)==50&&level(561)==75&&level(578)==100&&level(530)==25&&level(1023)==-1);puts("PASS: RUPTURE full icon threshold and invalid ADC guard");return 0;}
