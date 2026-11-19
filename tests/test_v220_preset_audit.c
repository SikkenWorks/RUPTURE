#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/etch-v1.88-source/firmware/fm1-native/rupture_preset_audit.h"

static struct ru_preset_audit_sig make(uint16_t pressure,uint8_t sub,
                                       uint16_t distortion){
 struct ru_preset_audit_sig sig={ru_audit_begin(),ru_audit_begin()};
 sig.core=ru_audit_u16(sig.core,pressure);
 sig.core=ru_audit_u16(sig.core,sub);
 sig.fx=ru_audit_u16(sig.fx,distortion);
 return sig;
}
int main(void){
 struct ru_preset_audit_sig a=make(24000u,1u,16000u);
 struct ru_preset_audit_sig b=make(24000u,1u,16000u);
 assert(ru_audit_equal(a,b));
 b=make(24001u,1u,16000u);
 assert(!ru_audit_equal(a,b) && a.fx==b.fx);
 b=make(24000u,1u,16001u);
 assert(!ru_audit_equal(a,b) && a.core==b.core);
 b=make(24000u,2u,16000u);
 assert(!ru_audit_equal(a,b));
 uint8_t bytes[]={0x34u,0x12u,0x78u,0x56u};
 uint32_t h=ru_audit_raw(ru_audit_begin(),bytes,sizeof(bytes));
 uint32_t explicit_hash=ru_audit_u16(ru_audit_u16(ru_audit_begin(),0x1234u),0x5678u);
 assert(h==explicit_hash);
 assert(ru_audit_u32(ru_audit_begin(),0x56781234u)==explicit_hash);
 puts("PASS V2.0.20 preset parameter audit: stable exact fields / independent core + FX");
 return 0;
}
