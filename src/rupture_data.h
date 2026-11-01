/* SPDX-License-Identifier: GPL-3.0-only
 * Host-verified data model: 12 projects * 8 sequence slots.
 * NEVER write until flash-sector ownership has been audited on FM1.
 */
#ifndef RUPTURE_DATA_H
#define RUPTURE_DATA_H
#include <stdint.h>
#define RUPTURE_PROJECTS 12
#define RUPTURE_SEQ_SLOTS 8
#define RUPTURE_STEPS 64
#define RUPTURE_STORAGE_MAGIC 0x52555054u /* RUPT */
#define RUPTURE_STORAGE_VERSION 1
/* Compact monophonic note data; 255 denotes REST. */
typedef struct {uint8_t note, velocity, length, flags;} RuptureStep;
typedef struct {uint8_t length,rate,swing,gate;RuptureStep step[RUPTURE_STEPS];} RuptureSequence;
typedef struct {char name[16];uint8_t root,scale,keylock,guide;uint8_t active_sequence;uint8_t params[16];RuptureSequence sequence[RUPTURE_SEQ_SLOTS];} RuptureProject;
typedef struct {uint32_t magic,version,serial,crc;RuptureProject project[RUPTURE_PROJECTS];} RuptureLibrary;
/* A 16-byte header is validated BEFORE any project read or write.
 * Device writes require: explicit opt-in + audited regions + power-safe journal.
 */
uint32_t rupture_crc32(const void *data,uint32_t size);
void rupture_library_init(RuptureLibrary *db);
void rupture_library_seal(RuptureLibrary *db);
int rupture_library_valid(const RuptureLibrary *db);
#endif
