/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stddef.h>

#include <errno.h>
#include <string.h>

#include <drivers/driver.h>
#include <drivers/major.h>
#include <initcall.h>

#include "types.h"

enum memdev_type {
    MEMDEV_NULL = 1,
    MEMDEV_ZERO,
    MEMDEV_FULL,
    MEMDEV_PORT,
    MEMDEV_RANDOM,
    MEMDEV_URANDOM,
};

static int null_read(off_t pos, char *buf, size_t n)
{
    return 0;
}

static int null_write(off_t pos, const char *buf, size_t n)
{
    return n;
}

static int zero_read(off_t pos, char *buf, size_t n)
{
    memset(buf, 0, n);
    return n;
}

static int full_write(off_t pos, const char *buf, size_t n)
{
    return -ENOSPC;
}

/* Architecture specific. */
int port_read(off_t, char *, size_t);
int port_write(off_t, const char *, size_t);

static int urandom_read(off_t pos, char *buf, size_t n)
{
    /*
     * Linear congruential generator.
     * See: https://stackoverflow.com/a/3062783
     * (Seed chosen by fair roll of the dice.)
     */
    /* TODO: Better RNG; True RNG (seed with keyboard inputs) */
    static int seed = 62521;
    for (size_t i = 0; i < n; i++) {
        seed = (9735 * seed + 13997) % 19468;
        *(buf++) = (char)seed;
    }
    return n;
}

int memdev_read(dev_t dev, off_t pos, char *buf, size_t n)
{
    switch (MINOR(dev))
    {
    case MEMDEV_NULL:
        return null_read(pos, buf, n);
    
    case MEMDEV_FULL:
    case MEMDEV_ZERO:
        return zero_read(pos, buf, n);
    
    case MEMDEV_PORT:
        return port_read(pos, buf, n);
    
    case MEMDEV_URANDOM:
        return urandom_read(pos, buf, n);
    
    default:
        return -ENODEV;
    }
}

int memdev_write(dev_t dev, off_t pos, const char *buf, size_t n)
{
    switch (MINOR(dev))
    {
    case MEMDEV_NULL:
    case MEMDEV_ZERO:
    case MEMDEV_URANDOM:
        return null_write(pos, buf, n);
    
    case MEMDEV_FULL:
        return full_write(pos, buf, n);
    
    case MEMDEV_PORT:
        return port_write(pos, buf, n);
    
    default:
        return -ENODEV;
    }
}

struct char_driver memdev_driver = {
    .read = memdev_read,
    .write = memdev_write,
};

void register_memdev_driver()
{
    register_char_driver(CHR_MEMDEV, &memdev_driver);
}

initcall(register_memdev_driver);
