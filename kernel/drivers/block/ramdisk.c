/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <drivers/block/ramdisk.h>

#include <stddef.h>

#include <errno.h>
#include <string.h>

#include <drivers/driver.h>
#include <drivers/major.h>
#include <initcall.h>

#include "types.h"

#define MAX_RAMDISK 8

struct ramdisk {
    blksize_t blksize;
    char *bufs;
    size_t size;
};

static struct ramdisk ramdisks[MAX_RAMDISK];

static int rd_idx = 0;

dev_t add_ramdisk(blksize_t blksize, void *bufs, size_t size)
{
    if (rd_idx >= MAX_RAMDISK)
        return NULL_DEV;
    
    ramdisks[rd_idx].blksize = blksize;
    ramdisks[rd_idx].bufs = bufs;
    ramdisks[rd_idx].size = size;

    dev_t ret = DEV(BLK_RAMDISK, rd_idx);

    rd_idx++;

    return ret;
}

blksize_t ramdisk_getblksize(dev_t dev)
{
    dev_t m = MINOR(dev);
    if (m >= MAX_RAMDISK)
        return 0;
    if (ramdisks[m].bufs == NULL)
        return 0;
    return ramdisks[m].blksize;
}

int ramdisk_readblk(dev_t dev, blkcnt_t blk, char *buf)
{
    dev_t m = MINOR(dev);

    if (m >= MAX_RAMDISK)
        return -ENODEV;
    if (ramdisks[m].bufs == NULL)
        return -ENODEV;
    
    blksize_t blksize = ramdisks[m].blksize;
    
    if (blk > ramdisks[m].size / blksize)
        return -EFAULT;

    memcpy(buf, &ramdisks[m].bufs[blk * blksize], blksize);
    return blksize;
}

int ramdisk_writeblk(dev_t dev, blkcnt_t blk, const char *buf)
{
    dev_t m = MINOR(dev);

    if (m >= MAX_RAMDISK)
        return -ENODEV;
    if (ramdisks[m].bufs == NULL)
        return -ENODEV;
    
    blksize_t blksize = ramdisks[m].blksize;

    memcpy(&ramdisks[m].bufs[blk * blksize], buf, blksize);
    return blksize;
}

struct block_driver ramdisk_driver = {
    .getblksize = ramdisk_getblksize,
    .readblk = ramdisk_readblk,
    .writeblk = ramdisk_writeblk,
};

void register_ramdisk_driver()
{
    register_block_driver(BLK_RAMDISK, &ramdisk_driver);
}

initcall(register_ramdisk_driver);
