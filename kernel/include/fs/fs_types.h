#ifndef FS_TYPES_H
#define FS_TYPES_H

#include <stdint.h>

enum i_perms {
    I_XUSR = 0001,
    I_WUSR = 0002,
    I_RUSR = 0004,
    I_RWXU = 0007,

    I_XGRP = 0010,
    I_WGRP = 0020,
    I_RGRP = 0040,
    I_RWXG = 0070,

    I_XOTH = 0100,
    I_WOTH = 0200,
    I_ROTH = 0400,
    I_RWXO = 0700,

    I_PERMS = 0777,
};

enum i_type {
    IT_UNK,

    IT_REG = 01000,
    IT_DIR = 02000,
    IT_BLK = 03000,
    IT_CHR = 04000,
    IT_FIFO = 05000,
    IT_LNK = 06000,
    IT_SOCK = 07000,

    IT_TYPE = 07000,
};

typedef uint16_t nlink_t;

typedef uint16_t mode_t;

typedef uint32_t ino_t;

typedef uint32_t dev_t;

typedef uint32_t uid_t;
typedef uint32_t gid_t;

typedef uint64_t off_t;

typedef uint32_t blksize_t;
typedef uint32_t blkcnt_t;

#endif
