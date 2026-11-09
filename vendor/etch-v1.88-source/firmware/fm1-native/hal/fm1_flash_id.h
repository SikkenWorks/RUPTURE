/* SPDX-License-Identifier: GPL-3.0-only
 * RUPTURE FM1 SPI NOR compatibility: manufacturer/device/capacity JEDEC IDs.
 * Both known 1 MiB parts implement the standard 0x9F, 0x05, 0x06, 0x20,
 * 0x02 commands used by the existing flash HAL. No change in flash layout.
 */
#pragma once
#include <stdint.h>
#define FL_JEDEC_P25Q80  0x856014u
#define FL_JEDEC_UC25HQ80 0xB36014u
#define FL_JEDEC_OK(id) ((uint32_t)(id) == FL_JEDEC_P25Q80 || (uint32_t)(id) == FL_JEDEC_UC25HQ80)
