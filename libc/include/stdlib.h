/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef __STDLIB_H
#define __STDLIB_H

/* NULL, size_t */
#include <stddef.h>

#ifdef __is_kernel

void *malloc(size_t size);
void free(void *ptr);

#endif

#endif
