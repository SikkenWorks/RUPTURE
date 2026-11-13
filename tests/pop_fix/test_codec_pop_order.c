/* Host-test RUPTURE pop-fix: volgorde in fm1_audio_init().
 * Bouwen via run_pop_tests.sh (die maakt een kopie van hal/fm1_audio.h met
 * de ALNK-registeradressen omgeleid naar host-variabelen). */
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
uint32_t host_port[8][8];
uint16_t host_con0, host_con1, host_len; uint8_t host_con2, host_con3;
uint32_t host_adr3, host_clk2, host_iomap5;
static uint32_t now_ms, clock_start_ms = UINT32_MAX, line_ms[8];
static int first_dma_bytes_nonzero;
static int32_t dma[512];
int host_line_before_dma;
void host_delay(uint32_t ms) { now_ms += ms; }
#include "fm1_audio_host.h"
static void isr(void) {}
static void track(void) {}
#define PC_OUT host_port[FM1_PC][FM1_OUT / 4u]
#define PC_DIR host_port[FM1_PC][FM1_DIR / 4u]
int main(void) {
    /* Koude start: PC0/1/2/6 nog ingang (DIR=1), uitgang laag. */
    PC_DIR = 0xFFu; PC_OUT = 0u;
    for (int i = 0; i < 512; i++) if (dma[i]) first_dma_bytes_nonzero = 1;
    assert(!first_dma_bytes_nonzero);
    /* Instrumentatie: we draaien init en controleren eindtoestand + volgorde
     * door de lijnen na de ALNK-start af te lezen. */
    host_con0 = 0;
    fm1_audio_init(dma, 256, isr, 3);
    (void)track; (void)clock_start_ms; (void)line_ms;
#if FM1_AUDIO_LEGACY_CODEC_ORDER
    assert(host_line_before_dma == 1);  /* oude volgorde: lijnen vóór I2S-klok */
#else
    assert(host_line_before_dma == 0);  /* fix: lijnen pas na I2S/DMA-start */
#endif
    assert(host_con0 & 0x800u);                       /* DMA/I2S loopt */
    assert((PC_OUT & 0x7u) == 0x7u && (PC_DIR & 0x7u) == 0u); /* codec-lijnen hoog */
    assert((PC_OUT & 0x40u) == 0u && (PC_DIR & 0x40u) == 0u);  /* PC6 LAAG uitgang */
#if !FM1_AUDIO_LEGACY_CODEC_ORDER
    assert(now_ms == 5u + 5u + FM1_AUDIO_CODEC_STEP_MS + FM1_AUDIO_CODEC_SETTLE_MS);
    /* Warme herstart: lijnen al hoog -> geen extra vertraging of flank. */
    now_ms = 0; host_con0 = 0;
    fm1_audio_init(dma, 256, isr, 3);
    assert(now_ms == 10u);
    assert((PC_OUT & 0x7u) == 0x7u && (PC_OUT & 0x40u) == 0u);
#else
    assert(now_ms == 10u);
#endif
#if FM1_AUDIO_LEGACY_CODEC_ORDER
    puts("PASS (legacy A/B-build) oude volgorde exact reproduceerbaar");
    return 0;
#endif
    puts("PASS fm1_audio_init: codec-lijnen na I2S-start, PC6 laag, warme herstart zonder extra flank");
    return 0;
}
