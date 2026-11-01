/* SPDX-License-Identifier: GPL-3.0-only
 * Pure read-only classification, never calls flash drivers.
 */
#ifndef RUPTURE_FIRST_BOOT_H
#define RUPTURE_FIRST_BOOT_H
#include <stdint.h>
typedef enum {RUPTURE_STORAGE_EMPTY=0,RUPTURE_STORAGE_OWNED=1,RUPTURE_STORAGE_FOREIGN=2} RuptureStorageKind;
RuptureStorageKind rupture_classify_header(const uint8_t *data,uint32_t length);
/* Format authorization requires two independent YES actions plus a verified
 * sector map. This helper never erases or writes anything. */
int rupture_format_authorized(int yes_initial,int yes_confirmed,int verified_whitelist,int protected_overlap);
#endif
