#!/usr/bin/env python3
"""Assert that V2.0.37 touched only UI arbitration, never audible DSP."""
from pathlib import Path
import hashlib

root=Path(__file__).resolve().parents[1]
src=root/'vendor/etch-v1.88-source/firmware/fm1-native'
sha=lambda data:hashlib.sha256(data).hexdigest()

# These target-source hashes are also independently pinned by the historic
# FX, CXS, USB and DMA tests. V2.0.37 MUST not change any of those modules.
unchanged={
 'native_cxs_q15.c':'86f4a39ad9bdbd6eabb79a34d9453e0fb0e150936667806e94eb2f00b57ca18f',
 'audio_core.c':'054b3791d142d998885039af1a362a0332d4ba9f6d538f47024c8548b830b7f5',
 'native_fx.c':'d7a7b55f99011d92a98f8ce8f20a6d2ec3374fc8f67fdcac48c70a05d248412a',
 'audio_target.c':'3813d7ac1b2d62c4c57a8a2221cf4cb7224bacb4ae79104995dc227a87dfabf6',
 'usb.c':'7992a47f973b73844477af454c6401063415964709645a68cf563aac5cd62d34',
}
for name, expected in unchanged.items():
 actual=sha((src/name).read_bytes())
 assert actual==expected,(name,actual,expected)

# Reverse EXACTLY the three deliberate UI-only edits. The resulting source
# must be identical to the V2.0.36 release. This guards every other control,
# note, storage, OTA, ISR, FX and timing branch in the huge bringup module.
text=(src/'bringup.c').read_text()
changes=[
 (''' /* Mirror note identity only in foreground. MIDI note edges may not touch
  * the physical FM1 key matrix, but still require LCD/FX arbitration. */
 uint32_t ru_last_note_serial=native_audio.alloc_serial;
 uint8_t ru_last_note_stack=native_audio.mono_count;
 uint8_t ru_last_note_held=native_audio.voice[0].held;
''',''),
 ('''   uint32_t ru_note_serial=native_audio.alloc_serial;
   uint8_t ru_note_stack=native_audio.mono_count;
   uint8_t ru_note_held=native_audio.voice[0].held;
   if(notes_changed||ru_note_serial!=ru_last_note_serial||
      ru_note_stack!=ru_last_note_stack||ru_note_held!=ru_last_note_held)
    ru_note_paint_changed(&ru_note_paint,etch_ms);
   ru_last_note_serial=ru_note_serial;
   ru_last_note_stack=ru_note_stack;
   ru_last_note_held=ru_note_held;''',
  '''   if(notes_changed)ru_note_paint_changed(&ru_note_paint,etch_ms);'''),
 ('''      ru_note_paint_allow_fx(&ru_note_paint,etch_ms,last_ui_paint_ms,
        ru_ui_command,native_ui_pressure,
        ru_note_fx_active_count(&native_controls.fx_chain,
                                native_controls.fx_chain_active))){''',
  '''      ru_note_paint_allow(&ru_note_paint,etch_ms,last_ui_paint_ms,
        ru_ui_command,native_ui_pressure)){'''),
]
for new,old in changes:
 assert text.count(new)==1,'Note-edge UI-only edit differs from audited patch'
 text=text.replace(new,old,1)
assert sha(text.encode())=='a16bafe1b6b2e3b47da7299c03e9aacb23147f21e77e4a107fd53bf8fda61c84'
assert '#define RU_FX_SRAM_HOT 0' in (src/'native_fx.c').read_text()
print('PASS V2.0.37: all audible DSP source hashes and original bringup outside UI arbitration unchanged')
