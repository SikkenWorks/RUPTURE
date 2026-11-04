#!/usr/bin/env python3
"""Apply verified, 32-bit exact-Q15 fastpaths to the RUPTURE V1.78 DSP."""
from pathlib import Path
import re
native=Path(__file__).resolve().parents[1]/'vendor/etch-v1.88-source/firmware/fm1-native'
p=native/'rupture_subosc.h';s=p.read_text()
a=s.index('static inline uint32_t ru_sub_inc(')
b=s.index('static inline int16_t ru_sub_sine_read(',a)
new='''/* V1.79: exact, saturating fixed-point increment, using TWO 32x32
 * multiplies instead of a 64-bit multiply per sample per active SUB.
 * factor is 0..131071 for ALL legal detune and drift settings (the
 * drift integrator is bounded to +/-768 Q16). The exceptional corrupted
 * state follows the original 64-bit calculation rather than truncating.
 * Unlike a 32-sample update this preserves EVERY sample's pitch, including
 * very slow drift, low notes and octave=+1. */
static inline uint32_t ru_sub_inc(uint32_t base,uint8_t octave,uint8_t detune,int32_t drift){
 uint64_t raw=base;
 if(octave==0u)base>>=2u;
 else if(octave==1u)base>>=1u;
 else if(octave==3u)raw<<=1u;
 int32_t cents=(int32_t)detune-50;
 int32_t signed_factor=65536+cents*38+drift;
 if(signed_factor<32768)signed_factor=32768;
 uint32_t factor=(uint32_t)signed_factor;
 if(factor>=131072u){
  /* Invalid-state defensive path. Regular musical settings never use it. */
  uint64_t v=(octave==3u)?raw:(uint64_t)base;
  v=(v*factor)>>16;
  return (uint32_t)(v>UINT32_MAX?UINT32_MAX:v<1u?1u:v);
 }
 uint32_t fraction=factor&65535u;
 uint32_t lo=(base&65535u)*fraction;
 uint32_t middle=(base>>16)*fraction;
 uint32_t part=lo>>16;
 if(middle>UINT32_MAX-part)return UINT32_MAX;
 uint32_t result=middle+part;
 if(factor>=65536u){
  if(result>UINT32_MAX-base)return UINT32_MAX;
  result+=base;
 }
 if(octave==3u){
  /* Exact floor((base * factor * 2) / 65536): include bit 15 of
   * product's low half. Saturate instead of overflowing the octave. */
  if(result>0x7fffffffu)return UINT32_MAX;
  result=(result<<1u)|((lo>>15u)&1u);
 }
 return result?result:1u;
}
'''
s=s[:a]+new+s[b:];p.write_text(s)

p=native/'native_fx.c';s=p.read_text()
pos=s.index('void etch_fx_reset(')
helper='''/* V1.79: the input/output of the normal FX signal path are bounded
 * Q15, so (wet-dry)*mix is ALWAYS in signed int32:
 * 65,535 * 32,767 = 2,147,385,345 < 2^31. For the uncommon unclipped
 * intermediate use the ORIGINAL 64-bit expression, preserving PCM exactly.
 * This is a CPU fastpath, not a reduced-precision wet mix. */
static inline int32_t ru_fx_delta_mix(int32_t dry,int32_t wet,int32_t mix){
 if(dry>=-32768 && dry<=32767 && wet>=-32768 && wet<=32767 &&
    mix>=0 && mix<=32767){
  return dry+((wet-dry)*mix)/32767;
 }
 return dry+(int32_t)((((int64_t)wet-(int64_t)dry)*mix)/32767);
}
static inline int32_t ru_fx_reverb_lerp(int32_t dry,int32_t wet,uint16_t space){
 /* Convex weighted sum of two Q15 samples cannot overflow int32. The
  * exact old numerator and truncation-to-zero division are preserved. */
 if(dry>=-32768 && dry<=32767 && wet>=-32768 && wet<=32767){
  return (dry*(int32_t)(32767u-space)+wet*(int32_t)space)/32767;
 }
 return (int32_t)(((int64_t)dry*(32767u-space)+
                    (int64_t)wet*space)/32767);
}
'''
s=s[:pos]+helper+s[pos:]
# Actual shared reverb output interpolation (both channels, exactly same samples).
old='''output=(int32_t)(((int64_t)dry_left*(32767u-space)+
                            (int64_t)wet_left*space)/32767);
 output_right=(int32_t)(((int64_t)dry_right*(32767u-space)+
                                  (int64_t)wet_right*space)/32767);'''
new='''output=ru_fx_reverb_lerp(dry_left,wet_left,space);
 output_right=ru_fx_reverb_lerp(dry_right,wet_right,space);'''
assert old in s
s=s.replace(old,new,1)
# Other global FX are simultaneously available on the MAX stress patch.
# Replace only the mathematically identical Q15 'x += wet difference' forms.
pattern=re.compile(r'(?P<indent>^[ ]*)(?P<base>x|output|output_right)\+=\(int32_t\)\(\(\(int64_t\)\((?P<wet>[A-Za-z_][A-Za-z_0-9]*)-(?P=base)\)\*(?P<mix>s->[A-Za-z_][A-Za-z_0-9]*)\)/32767\);',re.M)
items=[]
def repl(m):
 items.append((m.group('base'),m.group('wet'),m.group('mix')))
 return f"{m.group('indent')}{m.group('base')}=ru_fx_delta_mix({m.group('base')},{m.group('wet')},{m.group('mix')});"
s=pattern.sub(repl,s)
print('delta mixes accelerated:',items)
assert len(items)>=10, len(items)
p.write_text(s)
