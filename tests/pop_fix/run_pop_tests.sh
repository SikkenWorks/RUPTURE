#!/usr/bin/env bash
# Host-tests voor de RUPTURE pop-fix. Vereist gcc + python3.
set -euo pipefail
here="$(cd "$(dirname "$0")" && pwd)"
tmp="$(mktemp -d)"; trap 'rm -rf "$tmp"' EXIT
cp "$here"/host_stubs/*.h "$tmp"/
# Kopie van de echte HAL met ALNK/CLK/IOMAP-adressen omgeleid naar host-variabelen,
# en een order-probe die vastlegt of de codec-lijnen vóór/na de DMA-start hoog gaan.
python3 - "$here/../../vendor/etch-v1.88-source/firmware/fm1-native/hal/fm1_audio.h" "$tmp/fm1_audio_host.h" <<'PY'
import re,sys
s=open(sys.argv[1]).read()
rep={r'\(\*\(volatile uint16_t \*\)\(FM1_ALNK \+ 0x00u\)\)':'host_con0',
     r'\(\*\(volatile uint16_t \*\)\(FM1_ALNK \+ 0x04u\)\)':'host_con1',
     r'\(\*\(volatile uint8_t \*\)\(FM1_ALNK \+ 0x08u\)\)':'host_con2',
     r'\(\*\(volatile uint8_t \*\)\(FM1_ALNK \+ 0x0Cu\)\)':'host_con3',
     r'\(\*\(volatile uint32_t \*\)\(FM1_ALNK \+ 0x1Cu\)\)':'host_adr3',
     r'\(\*\(volatile uint16_t \*\)\(FM1_ALNK \+ 0x20u\)\)':'host_len',
     r'\(\*\(volatile uint32_t \*\)0x10014u\)':'host_clk2',
     r'\(\*\(volatile uint32_t \*\)0x51030u\)':'host_iomap5'}
for k,v in rep.items():
    s,n=re.subn(k,v,s); assert n==1,k
# Probe: bij elke hoog-zetting van PC0/1/2 vastleggen of DMA al liep.
s=s.replace('static void fm1__pc_out(uint32_t bit, int v)   /* codec control lines on PC0/1/2/6 */\n{',
 'extern int host_line_before_dma;\nstatic void fm1__pc_out(uint32_t bit, int v)   /* codec control lines on PC0/1/2/6 */\n{\n    if (v && bit < 3u && !(host_con0 & 0x800u)) host_line_before_dma = 1;',1)
open(sys.argv[2],'w').write(s)
PY
for legacy in 0 1; do
  gcc -std=c11 -Wall -Wextra -Werror -Wno-unused-function -DFM1_AUDIO_LEGACY_CODEC_ORDER=$legacy \
      -I"$tmp" -o "$tmp/t$legacy" "$here/test_codec_pop_order.c"
  "$tmp/t$legacy"
done
gcc -std=c11 -Wall -o "$tmp/startup" "$here/test_native_startup_pop.c"
"$tmp/startup"
gcc -std=c11 -Wall -Wextra -Werror -o "$tmp/gate" "$here/test_pop_led_gate.c"
"$tmp/gate"
