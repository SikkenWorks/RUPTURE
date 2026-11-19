/* RUPTURE preset diagnosis. Foreground only: the PCM ISR never hashes a
 * patch, reads NOR, or performs additional audio-rate work. Keep values
 * explicitly ordered so hashes never depend on host byte order. */
#ifndef RUPTURE_PRESET_AUDIT_H
#define RUPTURE_PRESET_AUDIT_H
#include <stdint.h>
struct ru_preset_audit_sig { uint32_t core,fx; };
static inline uint32_t ru_audit_begin(void){return 2166136261u;}
static inline uint32_t ru_audit_byte(uint32_t h,uint8_t b){return (h^(uint32_t)b)*16777619u;}
static inline uint32_t ru_audit_u16(uint32_t h,uint32_t v){
 h=ru_audit_byte(h,(uint8_t)v);return ru_audit_byte(h,(uint8_t)(v>>8u));
}
static inline uint32_t ru_audit_u32(uint32_t h,uint32_t v){
 h=ru_audit_u16(h,v);return ru_audit_u16(h,v>>16u);
}
static inline uint32_t ru_audit_raw(uint32_t h,const void *p,uint32_t size){
 const uint8_t *b=(const uint8_t*)p;
 for(uint32_t i=0u;i<size;i++)h=ru_audit_byte(h,b[i]);
 return h;
}
static inline uint8_t ru_audit_equal(struct ru_preset_audit_sig a,struct ru_preset_audit_sig b){
 return (uint8_t)(a.core==b.core && a.fx==b.fx);
}
#endif
