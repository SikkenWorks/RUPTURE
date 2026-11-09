#!/usr/bin/env python3
"""Minimal V1.85.2 compatibility patch based on V1.85.1 full source.

Explicitly recognize the two observed 1 MiB FM1 flash chips. Keep the audited
JieLi app-only loader untouched: it has no separate JEDEC allowlist, per
Felucca maintainer and local binary inspection.
"""
from pathlib import Path
r=Path(__file__).resolve().parents[1]
s=r/'vendor/etch-v1.88-source/firmware/fm1-native'
def replace(path, before, after):
 p=s/path
 old=p.read_text(); assert before in old,(path,before)
 assert old.count(before)==1,(path,old.count(before))
 p.write_text(old.replace(before,after,1))

(s/'hal/fm1_flash_id.h').write_text('''/* SPDX-License-Identifier: GPL-3.0-only
 * RUPTURE FM1 SPI NOR compatibility: manufacturer/device/capacity JEDEC IDs.
 * Both known 1 MiB parts implement the standard 0x9F, 0x05, 0x06, 0x20,
 * 0x02 commands used by the existing flash HAL. No change in flash layout.
 */
#pragma once
#include <stdint.h>
#define FL_JEDEC_P25Q80  0x856014u
#define FL_JEDEC_UC25HQ80 0xB36014u
#define FL_JEDEC_OK(id) ((uint32_t)(id) == FL_JEDEC_P25Q80 || (uint32_t)(id) == FL_JEDEC_UC25HQ80)
''')
replace(Path('hal/fm1_flash.h'), '#include "fm1_xip.h"', '#include "fm1_xip.h"\n#include "fm1_flash_id.h"')
replace(Path('hal/fm1_flash.h'), 'static RAMFN uint32_t fl_jedec_ram(void)                 /* expect 0x856014 */', 'static RAMFN uint32_t fl_jedec_ram(void)                 /* ID is checked against both FM1 parts */')
replace(Path('wave_store_target.c'), 'static uint8_t native_slot,native_save_confirm;', '''/* Cached before audio starts. Zero means the read was unavailable, NOT an
 * unsupported ID; in that case preserve V1.85.1's previously working writes.
 * A readable but unknown chip is never sent erase/program commands. */
static uint32_t native_flash_jedec_cached;
static uint8_t native_flash_jedec_recognized;
static int native_flash_write_allowed(void){
 return native_flash_jedec_cached==0u || native_flash_jedec_recognized;
}
static uint8_t native_slot,native_save_confirm;''')
p=s/'wave_store_target.c'
old=p.read_text()
before='if(!ETCH_NATIVE_STORAGE||!etch_recovery_may_write_store(native_recovery_reason))return -1;'
after='if(!ETCH_NATIVE_STORAGE||!etch_recovery_may_write_store(native_recovery_reason)||!native_flash_write_allowed())return -1;'
assert old.count(before)==2,old.count(before)
p.write_text(old.replace(before,after))
replace(Path('wave_store_target.c'), 'if(!ETCH_NATIVE_STORAGE||!etch_recovery_may_write_store(native_recovery_reason)||\n    (off&4095u)', 'if(!ETCH_NATIVE_STORAGE||!etch_recovery_may_write_store(native_recovery_reason)||!native_flash_write_allowed()||\n    (off&4095u)')
replace(Path('wave_store_target.c'), '''static void native_library_boot(void){
 if(!ETCH_NATIVE_STORAGE)return;
 native_store_status''', '''static void native_library_boot(void){
 if(!ETCH_NATIVE_STORAGE)return;
 /* The SPI NOR JEDEC read is a short, read-only, main-thread operation.
  * Cache once while sound/PC6 is muted, never in an IRQ or while playing. */
 fm1_irq_off();
 native_flash_jedec_cached=FL_FAR(fl_jedec_ram)();
 fm1_irq_enable_all();
 native_flash_jedec_recognized=(uint8_t)FL_JEDEC_OK(native_flash_jedec_cached);
 native_store_status''')
replace(Path('ota_target.c'), 'if(!ETCH_NATIVE_OTA||!native_updating||!FL_IN(off,0x1000u,OTA_AREA,OTA_AREA+OTA_AREA_LEN)', 'if(!ETCH_NATIVE_OTA||!native_updating||!native_flash_write_allowed()||!FL_IN(off,0x1000u,OTA_AREA,OTA_AREA+OTA_AREA_LEN)')
replace(Path('ota_target.c'), 'if(!ETCH_NATIVE_OTA||!native_updating||!FL_IN(off,n,OTA_AREA,OTA_AREA+OTA_AREA_LEN)', 'if(!ETCH_NATIVE_OTA||!native_updating||!native_flash_write_allowed()||!FL_IN(off,n,OTA_AREA,OTA_AREA+OTA_AREA_LEN)')
replace(Path('ota.c'), '''    for (i = 0; i < sizeof id; i++)
        id[i] = i < sizeof ID - 1u ? (uint8_t)ID[i] : 0;
    ota_send_msg(0x11, id, sizeof id);''','''    for (i = 0; i < sizeof id; i++)
        id[i] = i < sizeof ID - 1u ? (uint8_t)ID[i] : 0;
    /* The Firmware Hub convention: bytes 24..26 contain read-only JEDEC ID.
     * Old installers still see the identical null-terminated FM-1 identity. */
    id[24] = (uint8_t)(native_flash_jedec_cached >> 16);
    id[25] = (uint8_t)(native_flash_jedec_cached >> 8);
    id[26] = (uint8_t)native_flash_jedec_cached;
    ota_send_msg(0x11, id, sizeof id);''')
print('Applied V1.85.2 dual-JEDEC support')
