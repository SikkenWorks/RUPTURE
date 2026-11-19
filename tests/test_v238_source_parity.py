#!/usr/bin/env python3
"""Verify V238 cannot alter CXS/FX samples and only instruments ALNK0."""
from pathlib import Path
import hashlib

root=Path(__file__).resolve().parents[1]
src=root/'vendor/etch-v1.88-source/firmware/fm1-native'
sha=lambda data:hashlib.sha256(data).hexdigest()

# These complete native modules, not just selected functions, must remain
# byte-identical to the audited V237 firmware.
for name,expected in {
 'native_cxs_q15.c':'86f4a39ad9bdbd6eabb79a34d9453e0fb0e150936667806e94eb2f00b57ca18f',
 'audio_core.c':'054b3791d142d998885039af1a362a0332d4ba9f6d538f47024c8548b830b7f5',
 'native_fx.c':'d7a7b55f99011d92a98f8ce8f20a6d2ec3374fc8f67fdcac48c70a05d248412a',
 'usb.c':'7992a47f973b73844477af454c6401063415964709645a68cf563aac5cd62d34',
}.items():
 actual=sha((src/name).read_bytes())
 assert actual==expected,(name,actual,expected)

# Reverse exactly the read-only ISR instrumentation. The resulting ISR file
# must match the V237 release byte-for-byte: no synth, DMA, FX, or USB edits.
audio=(src/'audio_target.c').read_text()
added=[
 '#include "rupture_note_dma_meter.h"\n',
 '''/* Only the ALNK0 interrupt writes these counters; foreground reads 32-bit
 * snapshots. They are intentionally cumulative across audio restarts. */
static volatile ru_note_dma_meter_t native_note_dma;
''',
 '''#ifdef RUPTURE_BUILD
  const uint32_t nd_old_keys=native_audio.keys;
  const uint32_t nd_old_serial=native_audio.alloc_serial;
  const uint8_t nd_old_stack=native_audio.mono_count;
  const uint8_t nd_old_held=native_audio.voice[0].held;
#endif
''',
 '''#ifdef RUPTURE_BUILD
  const uint8_t nd_edges=native_output_enabled?
    ru_note_dma_edge(nd_old_keys,native_audio.keys,nd_old_serial,
      native_audio.alloc_serial,nd_old_stack,native_audio.mono_count,
      nd_old_held,native_audio.voice[0].held):0u;
#endif
''',
 '''#ifdef RUPTURE_BUILD
   ru_note_dma_record(&native_note_dma,nd_edges,render_elapsed,
                      (uint32_t)(fm1_ticks()-start),1u);
#endif
''',
 '''#ifdef RUPTURE_BUILD
  ru_note_dma_record(&native_note_dma,nd_edges,render_elapsed,
                     measure.wall_ticks,measure.late);
#endif
''',
]
for snippet in added:
 assert audio.count(snippet)==1,'ISR instrumentation differs from audited insertion'
 audio=audio.replace(snippet,'',1)
assert sha(audio.encode())=='3813d7ac1b2d62c4c57a8a2221cf4cb7224bacb4ae79104995dc227a87dfabf6'
print('PASS V2.0.38 CXS/FX/USB exact source parity and ISR read-only instrumentation')
