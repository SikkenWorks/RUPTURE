from pathlib import Path
p=Path(__file__).resolve().parents[1]/'vendor/etch-v1.88-source/firmware/fm1-native/native_fx.c'
s=p.read_text()
s=s.replace(' const uint32_t p=s->reverb_pos++;',' const uint32_t p=s->reverb_pos;\n s->reverb_pos=(p+1u)&4095u;',1)
s=s.replace(' const uint32_t k0=s->reverb_line_cursor[0];\n const uint32_t k1=s->reverb_line_cursor[1];',' const uint32_t k0=s->reverb_line_cursor[0];\n const uint32_t k1=s->reverb_line_cursor[1];',1)
s=s.replace(' int32_t fb=18500+((int32_t)decay*10200>>15); /* 0.56 .. 0.876 */',' int32_t fb=s->reverb_spring_feedback; /* cached at knob updates */')
s=s.replace(' s->reverb_line_cursor[0]=(uint16_t)(k0+1u==2039u?0u:k0+1u);\n s->reverb_line_cursor[1]=(uint16_t)(k1+1u==2057u?0u:k1+1u);',' s->reverb_line_cursor[0]=(uint16_t)(k0+1u==s->tap1?0u:k0+1u);\n s->reverb_line_cursor[1]=(uint16_t)(k1+1u==s->tap2?0u:k1+1u);')
s=s.replace(' static const uint16_t sizes[4]={727,881,1013,1475};\n','')
s=s.replace(' const int32_t gain=20000+((int32_t)decay*10000>>15); /* 0.61..0.916 */',' const int32_t gain=s->reverb_room_feedback; /* cached at knob updates */')
s=s.replace('  s->reverb_line_cursor[i]=(uint16_t)(next==sizes[i]?0u:next);','  uint32_t length=i==0u?s->tap1:i==1u?s->tap2:i==2u?s->tap3:s->reverb_room_line4;\n  s->reverb_line_cursor[i]=(uint16_t)(next==length?0u:next);')
a=s.index('  switch(etch_fx_reverb_type(cfg)){',s.index('s->space_damp_alpha='));b=s.index('\n }\n uint16_t feedback=',a)
s=s[:a]+'''  switch(etch_fx_reverb_type(cfg)){
   case 0: /* SPRING: two independent coils in the existing 4096-word tank. */
    s->tap1=(uint16_t)(641u+span*1398u/32767u); /* max 2039 */
    s->tap2=(uint16_t)(803u+span*1254u/32767u); /* max 2057 */
    s->reverb_spring_feedback=(uint16_t)(18500u+(uint32_t)qclamp(cfg->reverb_decay)*10200u/32767u);
    break;
   case 2: /* ROOM: 4 independent delays, max capacities 727/881/1013/1475. */
    s->tap1=(uint16_t)(241u+span*486u/32767u);
    s->tap2=(uint16_t)(283u+span*598u/32767u);
    s->tap3=(uint16_t)(377u+span*636u/32767u);
    s->reverb_room_line4=(uint16_t)(489u+span*986u/32767u);
    s->reverb_room_feedback=(uint16_t)(20000u+(uint32_t)qclamp(cfg->reverb_decay)*10000u/32767u);
    break;
   default: /* HALL: preserve V1.79 timing, tone and PCM. */
    s->tap1=(uint16_t)(521u+span*727u/32767u);
    s->tap2=(uint16_t)(1471u+span*1201u/32767u);
    s->tap3=(uint16_t)(2897u+span*1133u/32767u);
    break;
  }
  /* A SIZE change can shorten a delay line while its cursor is beyond
   * the new bound. Clamp ONCE at the control update, not per sample. */
  if(etch_fx_reverb_type(cfg)==0u || etch_fx_reverb_type(cfg)==2u){
   uint16_t len[4]={s->tap1,s->tap2,s->tap3,s->reverb_room_line4};
   uint32_t count=etch_fx_reverb_type(cfg)==0u?2u:4u;
   for(uint32_t j=0;j<count;j++)if(s->reverb_line_cursor[j]>=len[j])s->reverb_line_cursor[j]=0u;
  }'''+s[b:]
p.write_text(s)
p=Path(__file__).resolve().parents[1]/'vendor/etch-v1.88-source/firmware/fm1-native/native_fx.h';s=p.read_text();s=s.replace(' uint8_t reverb_algorithm_seen;',' uint16_t reverb_room_line4,reverb_room_feedback,reverb_spring_feedback;');p.write_text(s)
print('Control-rate cached feedback, size-based segment lengths, bounded ring cursor')
