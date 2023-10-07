#ifndef DRIVER_H
#define DRIVER_H

#include <stddef.h>

#include "types.h"

struct char_driver {
    int (*read)(dev_t, off_t, char *, size_t);
    int (*write)(dev_t, off_t, const char *, size_t);
};

void register_char_driver(unsigned char major, struct char_driver *dr);
int read_char(dev_t dev, off_t pos, char *buf, size_t n);
int write_char(dev_t dev, off_t pos, const char *buf, size_t n);

struct block_driver {
    blksize_t (*getblksize)(dev_t);
    int (*readblk)(dev_t, blkcnt_t, char *);
    int (*writeblk)(dev_t, blkcnt_t, const char *);
};

void register_block_driver(unsigned char major, struct block_driver *dr);
blksize_t getblksize(dev_t dev);
int read_block(dev_t dev, blkcnt_t blk, char *buf);
int write_block(dev_t dev, blkcnt_t blk, const char *buf);

#endif
