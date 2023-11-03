/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef RAMDISK_H
#define RAMDISK_H

#include <stddef.h>

#include "types.h"

dev_t add_ramdisk(blksize_t blksize, void *bufs, size_t size);

#endif
