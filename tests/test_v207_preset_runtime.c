#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#define RU_PAGE_SETTINGS 16u
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_preset_input.h"

int main(void){
 assert(ru_preset_input_action(1,0u,0u,0u,0u,0u,16u)==RU_PRESET_INPUT_PARAMETER);
 assert(ru_preset_input_action(-5,0u,12u,0u,0u,0u,16u)==RU_PRESET_INPUT_PARAMETER);
 assert(ru_preset_input_action(1,0u,RU_PAGE_SETTINGS,0u,0u,0u,16u)==RU_PRESET_INPUT_PARAMETER);
 assert(ru_preset_input_action(1,0u,RU_PAGE_SETTINGS,1u,0u,0u,16u)==RU_PRESET_INPUT_PARAMETER);
 assert(ru_preset_input_action(1,0u,RU_PAGE_SETTINGS,2u,0u,0u,16u)==RU_PRESET_INPUT_NONE);
 assert(ru_preset_input_action(1,0u,RU_PAGE_SETTINGS,3u,0u,0u,16u)==RU_PRESET_INPUT_NONE);
 assert(ru_preset_input_action(1,1u<<11,RU_PAGE_SETTINGS,3u,0u,0u,16u)==RU_PRESET_INPUT_NONE);
 assert(ru_preset_input_action(3,1u<<11,0u,0u,0u,0u,16u)==RU_PRESET_INPUT_SEQ_ROTATE);
 assert(ru_preset_input_action(-3,1u<<11,12u,0u,0u,0u,128u)==RU_PRESET_INPUT_SEQ_ROTATE);
 assert(ru_preset_input_action(1,1u<<11,0u,0u,0u,0u,1u)==RU_PRESET_INPUT_PARAMETER);
 assert(ru_preset_input_action(0,0u,0u,0u,0u,0u,16u)==RU_PRESET_INPUT_NONE);
 assert(ru_preset_input_action(1,0u,0u,0u,1u,0u,16u)==RU_PRESET_INPUT_NONE);
 assert(ru_preset_input_action(1,0u,0u,0u,0u,1u,16u)==RU_PRESET_INPUT_NONE);
 puts("PASS V2.0.7 physical PRESETS detents -> parameter; SEQ chord -> pattern; modal guards");
 return 0;
}
