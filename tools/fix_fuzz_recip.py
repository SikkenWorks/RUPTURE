from pathlib import Path
p=Path(__file__).resolve().parents[1]/'vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c'
s=p.read_text()
arr=[0]*4+[round((1<<24)/(i*64)) for i in range(4,518)]
table='''/* V1.42 periodic fuzz makeup: 2 KiB flash-only reciprocal table.
 * wet_ref is 256..33023, so index is 4..516. No variable integer
 * division inside the audio IRQ; linear interpolation limits quantization.
 * Q24 reciprocal * dry_ref >> 9 produces Q15 output ratio. */
static const uint32_t rupture_fuzz_recip_q24[518]={\n'''
for i in range(0,len(arr),10):table+=' '+','.join(str(n) for n in arr[i:i+10])+',\n'
table+='};\n'
table+='''static inline int32_t rupture_fuzz_ratio_q15(int32_t dry_ref,int32_t wet_ref){
 if(dry_ref>=wet_ref)return 32767;
 uint32_t idx=(uint32_t)wet_ref>>6, frac=(uint32_t)wet_ref&63u;
 uint32_t a=rupture_fuzz_recip_q24[idx],b=rupture_fuzz_recip_q24[idx+1u];
 uint32_t recip=(uint32_t)((int32_t)a+(((int32_t)b-(int32_t)a)*(int32_t)frac)/64);
 int32_t ratio=(int32_t)(((uint32_t)dry_ref*recip)>>9);
 return ratio<4096?4096:ratio>32767?32767:ratio;
}
'''
needle='static inline int32_t rupture_fuzz_soft_rect(int32_t x){'
assert needle in s;s=s.replace(needle,table+needle,1)
needle='''  int32_t ratio=(dry_ref*32767)/wet_ref;
  if(ratio>32767)ratio=32767;
  if(ratio<4096)ratio=4096;
  s->rupture_fuzz_makeup_target=ratio;'''
new='''  s->rupture_fuzz_makeup_target=rupture_fuzz_ratio_q15(dry_ref,wet_ref);'''
assert needle in s;s=s.replace(needle,new,1)
p.write_text(s)
print('Generated reciprocal entries',len(arr))
