/* One active step chases sixteen white FM1 keys; page follows playhead. */
#pragma once
#include <stdint.h>
static inline uint8_t ru_white_step_key(uint16_t current){
 static const uint8_t map[16]={0,2,4,6,7,9,11,12,14,16,18,19,21,23,24,26};
 return map[current&15u];
}
static inline uint32_t ru_white_step_mask(uint16_t length,uint16_t active){
 if(length<1u || length>128u)return 0u;
 uint16_t start=(uint16_t)(active&~15u);
 uint32_t bits=0u;
 for(uint8_t i=0;i<16u;i++)if(start+i<length)bits|=(uint32_t)1u<<ru_white_step_key(i);
 return bits;
}
