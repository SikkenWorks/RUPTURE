#!/usr/bin/env python3
"""Execute production pause/start/store callbacks against a simulated DMA/HAL.
The physical codec, SPI noise and rail collapse are intentionally not modeled.
"""
from pathlib import Path
import os, re, subprocess, tempfile
ROOT=Path(__file__).resolve().parents[1]
N=ROOT/'vendor/etch-v1.88-source/firmware/fm1-native'
audio=(N/'audio_target.c').read_text()
store=(N/'wave_store_target.c').read_text()

def function(source,name):
    m=re.search(r'^static (?:void|int) '+name+r'\([^;]*?\)\{',source,re.M)
    assert m,name
    pos=m.end();depth=1
    # These functions have no braces in strings/comments.
    while depth:
        depth+=(source[pos]=='{')-(source[pos]=='}');pos+=1
    return source[m.start():pos]

# Integration ordering: the final fade must follow preset crossfade, and
# quiet completion must follow the real DMA post-copy timing check.
irq=audio.split('void fm1_alnk0_irq(void){',1)[1].split('static void native_start(void){',1)[0]
assert irq.index('etch_preset_ramp_frame(')<irq.index('etch_storage_audio_frame(')<irq.index('dst[2*i]=left*128')
assert irq.index('etch_audio_dma_preflight(')<irq.index('etch_storage_audio_frame(')
assert irq.index('etch_audio_timing_end(')<irq.index('etch_storage_audio_published(')
assert 'etch_storage_audio_published(&native_storage_audio,storage_zero,half)' in irq
assert 'native_storage_quiet_request&&!native_storage_audio.remaining' in irq
pause=function(audio,'native_pause')
assert 'fm1_audio_output_enable' not in pause and 'fm1_audio_stop' not in pause
start=function(audio,'native_start')
assert start.index('if(warm)return;')<start.index('native_dma[i]=0')<start.index('fm1_audio_init(')
for name in ('native_store_read','native_store_erase','native_store_program',
             'native_store_wipe_read','native_store_wipe_erase'):
    f=function(store,name)
    assert f.index('native_storage_audio_prepare()')<f.index('FL_FAR(')
main=(N/'bringup.c').read_text()
assert 'native_storage_read_poll((uint8_t)(!muted&&!native_tripped));' in main
assert 'native_storage_audio_failed=0u;native_start();' in main
assert '!native_updating && !native_storage_audio_failed)' in main
for name in ('ota_erase','ota_prog','ota_fread'):
    f=function((N/'ota_target.c').read_text(),name)
    assert 'if(native_storage_audio_failed||!native_suspended)return -8;' in f

prelude=r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "native_storage_audio.h"
#include "native_output_startup.h"
#include "native_preset_ramp.h"
#include "wave_store.h"
#define ETCH_NATIVE_FRAMES 128u
#define ETCH_SCOPE_CAPTURE 1024u
#define ETCH_NATIVE_PANEL_MIDI 0
#define ETCH_NATIVE_USB 0
#define ETCH_NATIVE_STORAGE 1
#define FL_FAR(x) x
#define FL_IN(off,n,lo,hi) ((off)>=(lo)&&(off)<=(hi)&&(n)<=(hi)-(off))
static uint8_t native_audio_clock_live,native_storage_quiet_request;
static volatile struct etch_storage_audio native_storage_audio;
static uint8_t native_storage_read_resume,native_storage_audio_failed;
static uint8_t native_suspended,native_output_enabled,native_boot_audio_released;
static uint8_t native_output_analog_ready,native_tripped;
static uint32_t native_missed_streak,native_healthy_blocks,native_ui_pressure;
static uint32_t native_render_wall_peak,native_publish_wall_peak,native_scope_deferred;
static uint32_t native_controls_wall_peak,native_midi_queue_peak,native_midi_deferred_batches;
static uint32_t native_midi_batch_peak,native_midi_backlog,native_min_dma_slack,native_last_dma_slack;
static uint32_t native_output_fade,native_prepared_blocks,native_halves,native_preset_ramp_requested;
static uint32_t native_scope_ready,native_scope_cursor,native_scope_has_signal,native_scope_held_note;
static uint32_t native_scope_window_peak,native_scope_silent_windows;
static int native_recovery_reason;
static struct etch_preset_ramp native_preset_ramp;
static struct etch_output_usb_gate native_usb_gate;
static struct etch_output_gate native_output_gate;
static struct etch_output_screen_gate native_screen_gate;
static int32_t native_dma[512];
static int16_t native_scope[2][1024];
static uint32_t native_audio,native_panel_out;
static unsigned irq_mask,clock_on,pc6,clock_stops,pc6_changes,inits,resets;
static unsigned blocks,half_index,stall,inject_late,flash_calls,flash_fail;
static uint32_t ticks;
static void native_start(void);
static void fm1_irq_off(void){irq_mask=1;}
static void fm1_irq_enable_all(void){irq_mask=0;}
static uint32_t fm1_ticks(void){return ticks;}
static void fm1_audio_ack_half(void){}
static void isr_alnk0(void){}
static void fm1_audio_stop(void){clock_on=0;++clock_stops;}
static void fm1_audio_output_enable(unsigned on){if(pc6!=on)++pc6_changes;pc6=on;}
static void fm1_audio_init(int32_t *d,unsigned words,void (*isr)(void),unsigned prio){
 assert(d==native_dma&&words==256&&isr==isr_alnk0&&prio==3);clock_on=1;++inits;
}
static void etch_native_reset(uint32_t *s){*s=0;++resets;}
static void etch_panel_midi_reset(uint32_t *s){*s=0;}
static void fm1_wdt_feed(void){
 ticks+=69632u;
 if(irq_mask||stall||!clock_on||native_suspended)return;
 if(native_storage_quiet_request){
  unsigned zero=!native_storage_audio.remaining;
  for(unsigned i=0;i<128;++i){
   int32_t l=24000,r=-18000;
   etch_storage_audio_frame(&native_storage_audio,&l,&r);
   native_dma[half_index*256+2*i]=l*128;
   native_dma[half_index*256+2*i+1]=r*128;
  }
  if(!inject_late)etch_storage_audio_published(&native_storage_audio,zero,half_index);
  half_index^=1;++blocks;++native_halves;
 }
}
static int etch_recovery_may_write_store(int x){(void)x;return 1;}
static int native_flash_write_allowed(void){return 1;}
static void assert_flash_safe(void){
 assert(irq_mask);
 if(clock_on){assert(native_suspended);for(unsigned i=0;i<512;++i)assert(native_dma[i]==0);}
 ++flash_calls;
}
static int fl_read_ram(uint32_t off,uint8_t *dst,uint32_t n){
 (void)off;assert_flash_safe();memset(dst,0x5a,n);return flash_fail?-7:0;
}
static int fl_erase4k_ram(uint32_t off,uint32_t *took){(void)off;(void)took;assert_flash_safe();return flash_fail?-7:0;}
static int fl_prog_ram(uint32_t off,const uint8_t *src,uint32_t n,uint32_t *took){
 (void)off;(void)src;(void)n;(void)took;assert_flash_safe();return flash_fail?-7:0;
}
'''
body='\n'.join(function(audio,name) for name in ('native_stop','native_pause','native_storage_audio_prepare','native_start','native_storage_read_poll'))
body+='\n'+'\n'.join(function(store,name) for name in ('native_store_read','native_store_erase','native_store_program','native_store_wipe_read','native_store_wipe_erase'))
tests=r'''
static void playing(void){
 stall=inject_late=flash_fail=0;
 native_suspended=native_tripped=native_storage_audio_failed=native_storage_read_resume=0;
 native_audio_clock_live=native_output_enabled=native_output_analog_ready=native_boot_audio_released=1;
 clock_on=pc6=1;pc6_changes=clock_stops=inits=resets=blocks=0;
 irq_mask=0;half_index=0;
 for(unsigned i=0;i<512;++i)native_dma[i]=900000;
}
int main(void){
 /* Every int16 amplitude, both signs, is monotone down to exact zero. */
 for(int value=-32768;value<=32767;++value){
  struct etch_storage_audio s;etch_storage_audio_begin(&s);
  int32_t prev=value<0?-value:value;
  for(unsigned i=0;i<256;++i){
   int32_t l=value,r=-value;etch_storage_audio_frame(&s,&l,&r);
   int32_t mag=l<0?-l:l;assert(mag<=prev);assert(l==-r);prev=mag;
  }
  assert(!prev&&!etch_storage_audio_ready(&s));
  etch_storage_audio_published(&s,0,0);assert(!etch_storage_audio_ready(&s));
  etch_storage_audio_published(&s,1,0);
  etch_storage_audio_published(&s,1,0); /* repeated half is NOT two halves */
  etch_storage_audio_published(&s,1,99);assert(!etch_storage_audio_ready(&s));
  etch_storage_audio_published(&s,1,1);assert(etch_storage_audio_ready(&s));
 }
 unsigned total_flash=0;
 for(unsigned trial=0;trial<10000;++trial){
  playing();ticks=trial&1?UINT32_MAX-140000u:0u;
  uint32_t old_halves=native_halves;
  native_pause();
  assert(native_suspended&&!native_output_enabled&&!native_tripped);
  assert(blocks==4&&native_halves==old_halves+4);
  assert(!clock_stops&&!pc6_changes&&clock_on&&pc6);
  for(unsigned i=0;i<512;++i)assert(!native_dma[i]);
  native_pause();assert(blocks==4); /* nested SAVE/load/rollback */
  uint8_t data[600];
  assert(!native_store_read(ETCH_STORE_BASE,data,sizeof data));
  for(unsigned i=0;i<600;++i)assert(data[i]==0x5a);
  assert(!native_store_erase(ETCH_STORE_BASE));
  assert(!native_store_program(ETCH_STORE_BASE,data,128));
  assert(!native_store_wipe_read(ETCH_STORE_BASE,data,1));
  assert(!native_store_wipe_erase(ETCH_STORE_BASE));
  flash_fail=1;assert(native_store_read(ETCH_STORE_BASE,data,1)==-7);flash_fail=0;
  assert(native_store_read(0,data,1)==-1); /* protected flash read refused */
  assert(native_store_wipe_erase(ETCH_STORE_BASE+1)==-1);
  native_start();assert(native_output_enabled&&!native_suspended&&!native_output_fade);
  assert(native_halves==old_halves+4&&!inits&&!clock_stops&&!pc6_changes);
  unsigned reset_count=resets;native_start();assert(resets==reset_count);
  ++total_flash;
 }
 /* Unwrapped catalog reads coalesce into ONE pause, one later resume. */
 playing();uint8_t data[300];
 for(unsigned i=0;i<40;++i)assert(!native_store_read(ETCH_STORE_BASE,data,2));
 assert(blocks==4&&native_storage_read_resume&&native_suspended);
 native_storage_read_poll(0);assert(native_suspended); /* muted user intent */
 native_storage_read_poll(1);assert(!native_suspended&&!native_storage_read_resume);
 assert(!pc6_changes&&!clock_stops&&!inits);
 /* A failed read still resumes safely without touching the analogue gate. */
 playing();flash_fail=1;assert(native_store_read(ETCH_STORE_BASE,data,2)==-7);
 native_storage_read_poll(1);assert(!native_suspended&&!pc6_changes);
 /* A stopped ISR or consistently rejected DMA publication blocks all I/O.
  * The bounded fault path is the ONLY hardware stop. */
 for(unsigned mode=0;mode<2;++mode){
  playing();stall=mode==0;inject_late=mode==1;ticks=UINT32_MAX-30000u;
  unsigned old=flash_calls;
  assert(native_store_read(ETCH_STORE_BASE,data,2)==-1);
  assert(native_storage_audio_failed&&native_tripped&&native_suspended);
  assert(!clock_on&&!pc6&&clock_stops==1&&flash_calls==old);
  native_start();assert(native_tripped&&!clock_on); /* cannot silently restart */
  native_storage_read_poll(1);assert(!clock_on);
 }
 /* Boot/midi-only reads neither start nor unmute an absent codec. */
 native_storage_audio_failed=0;native_audio_clock_live=0;native_suspended=1;
 native_tripped=0;native_storage_read_resume=0;
 assert(!native_store_read(ETCH_STORE_BASE,data,2));
 assert(!clock_on&&!native_storage_read_resume);
 /* A true cold start retains the original codec gate and zero DMA init. */
 stall=inject_late=0;native_start();
 assert(inits==1&&clock_on&&!native_output_enabled&&!pc6);
 puts("PASS storage audio: 65536 amplitude fades; 10000 warm transactions; both DMA halves; nested pauses; journal read coalescing; read errors; timeout/wrap/fail-closed; cold boot");
 return 0;
}
'''
with tempfile.TemporaryDirectory(prefix='rupture-v260-') as tmp:
    source=Path(tmp)/'test.c';source.write_text(prelude+'\n'+body+'\n'+tests)
    binary=Path(tmp)/'test'
    subprocess.run([os.environ.get('CC','gcc'),'-std=c11','-O2','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-fsanitize=address,undefined','-fno-sanitize-recover=all','-I'+str(N),str(source),'-o',str(binary)],check=True)
    subprocess.run([str(binary)],check=True)
print('PASS production source integration: PCM ordering, DMA timing, every raw store callback, OTA gating and manual fault rearm')
