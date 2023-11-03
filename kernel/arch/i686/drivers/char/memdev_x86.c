/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * file: kernel/arch/i686/drivers/char/memdev_x86.c
 *
 * Architecture specific memory devices (`/dev/port`).
 */

#include <stddef.h>
#include <stdint.h>

#include <errno.h>

#include <x86/pio.h>

#include "types.h"

int port_read(off_t pos, char *buf, size_t n)
{
    if (pos > UINT16_MAX)
        return -EFAULT;
    
    /* Read at most until the end of the I/O bus (port 0xffff). */
    uint16_t newn = UINT16_MAX - pos + 1;
    newn = newn < n ? newn : n;

    for (uint16_t i = 0; i < newn; i++) {
        *(buf++) = inb(pos++);
    }

    return newn;
}

int port_write(off_t pos, const char *buf, size_t n)
{
    if (pos > UINT16_MAX)
        return -EFAULT;
    
    /* Write at most until the end of the I/O bus (port 0xffff). */
    uint16_t newn = UINT16_MAX - pos + 1;
    newn = newn < n ? newn : n;

    for (uint16_t i = 0; i < newn; i++) {
        outb(pos++, *(buf++));
    }

    return newn;
}
