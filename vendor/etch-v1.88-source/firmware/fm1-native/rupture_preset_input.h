/* RUPTURE V2.0.7: pure control-loop dispatch for the physical PRESETS encoder.
 * Unlike the old ETCH fallback this runs before RUPTURE's early continue.
 * No audio, LED-scan ISR, flash, memory allocation or display operation. */
#ifndef RUPTURE_PRESET_INPUT_H
#define RUPTURE_PRESET_INPUT_H
#include <stdint.h>
enum RuPresetInputAction {
 RU_PRESET_INPUT_NONE=0,
 RU_PRESET_INPUT_PARAMETER=1,
 RU_PRESET_INPUT_SEQ_ROTATE=2
};
static inline enum RuPresetInputAction ru_preset_input_action(
    int32_t detents,uint32_t buttons,uint8_t screen,uint8_t settings_open,
    uint8_t saver,uint8_t protected_ui,uint8_t length){
 if(!detents||saver||protected_ui)return RU_PRESET_INPUT_NONE;
 if(screen==RU_PAGE_SETTINGS && settings_open>=2u)return RU_PRESET_INPUT_NONE;
 if((buttons&(1u<<11)) && length>1u)return RU_PRESET_INPUT_SEQ_ROTATE;
 return RU_PRESET_INPUT_PARAMETER;
}
#endif
