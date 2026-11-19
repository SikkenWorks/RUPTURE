from pathlib import Path

SRC=Path(__file__).resolve().parents[1]/'vendor/etch-v1.88-source/firmware/fm1-native'
fx=(SRC/'native_fx.c').read_text()
audio=(SRC/'audio_core.c').read_text()
chain=(SRC/'rupture_fx_chain.h').read_text()
assert 'static inline uint8_t ru_chain_span(' in chain
assert 'rupture_fx_process_ordered_compact(' in fx
assert 'for(uint8_t index=0;index<count;index++)' in fx
assert 'packed_slots?packed_slots[index]:index' in fx
assert 'rupture_fx_process_ordered_compact(s,sample,chorus,delay,reverb,mosfet' in fx
assert 'const struct rupture_fx_dispatch ru_fx_dispatch=' in audio
assert audio.index('const struct rupture_fx_dispatch ru_fx_dispatch=')<audio.index('for(uint32_t f=0;f<frames;f++)')
assert 'rupture_fx_process_ordered_fast(' in audio
assert '&c->fx_chain,c->fx_bypass_mask,&l,&r,&ru_fx_dispatch,&ru_fx_control_checked,' in audio
assert '&ru_distortion_coefficients);' in audio
assert 'ru_distortion_coefficients.valid_mask=0u;' in audio
assert 'rupture_fx_process_ordered_cached(' in fx
assert 'ru_fx_control_checked=0u;' in audio
assert 'if((f&31u)==0u){' in audio
assert '#ifdef RUPTURE_BUILD' in audio
print('PASS FX span compatibility and V2.0.22 packed wiring at block boundary')
