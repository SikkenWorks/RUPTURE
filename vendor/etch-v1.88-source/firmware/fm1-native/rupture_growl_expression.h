/* RUPTURE V2.0.14: integer GROWL performance return, UI service only. */
#ifndef RUPTURE_GROWL_EXPRESSION_H
#define RUPTURE_GROWL_EXPRESSION_H
#include <stdint.h>
#define RU_GROWL_EXPR_NEUTRAL 50u
#define RU_GROWL_EXPR_RETURN_DELAY_MS 125u
#define RU_GROWL_EXPR_RETURN_STEP 5u
static inline uint8_t ru_growl_expression_return_step(
 uint8_t value,uint32_t now_ms,uint32_t last_turn_ms){
 if(value==RU_GROWL_EXPR_NEUTRAL || !last_turn_ms ||
    (uint32_t)(now_ms-last_turn_ms)<RU_GROWL_EXPR_RETURN_DELAY_MS)return value;
 if(value>RU_GROWL_EXPR_NEUTRAL)
  return (uint8_t)(value>RU_GROWL_EXPR_NEUTRAL+RU_GROWL_EXPR_RETURN_STEP?
   value-RU_GROWL_EXPR_RETURN_STEP:RU_GROWL_EXPR_NEUTRAL);
 return (uint8_t)(value+RU_GROWL_EXPR_RETURN_STEP<RU_GROWL_EXPR_NEUTRAL?
   value+RU_GROWL_EXPR_RETURN_STEP:RU_GROWL_EXPR_NEUTRAL);
}
#endif
