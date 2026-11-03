/* RUPTURE V1.61: encoder polish, UI event loop only.
 * Preserve one detent/step at human pace and discrete choices always.
 * Sustained fast turns on continuous controls may advance 2 units per click.
 * No floats, heap, timers, audio/DSP or persistent schema changes.
 */
#ifndef RU_ENCODER_FEEL_H
#define RU_ENCODER_FEEL_H
#include <stdint.h>
typedef struct {uint32_t previous_ms;int8_t direction;uint8_t consecutive;} RuEncoderFeel;
static inline int32_t ru_encoder_feel(RuEncoderFeel *f,uint32_t now,int32_t clicks,uint8_t continuous){
 if(!clicks)return 0;
 const int8_t dir=(int8_t)(clicks>0?1:-1);
 /* Discrete options must follow physical click counts exactly. */
 if(!continuous){f->consecutive=0u;f->direction=0;f->previous_ms=now;return clicks;}
 uint32_t elapsed=(uint32_t)(now-f->previous_ms);
 if(f->direction!=dir || elapsed>100u || f->consecutive==0u)f->consecutive=1u;
 else if(f->consecutive<4u)++f->consecutive;
 f->direction=dir;f->previous_ms=now;
 /* A hardware burst already has several clicks: retain their count. */
 if(clicks>1 || clicks<-1)return clicks;
 return (f->consecutive>=3u && elapsed<=70u)?(int32_t)dir*2:clicks;
}
#endif
