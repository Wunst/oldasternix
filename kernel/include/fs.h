/**
 * @file kernel/include/fs.h
 * 
 * @brief Kernel filesystem driver interface
 */

#ifndef FS_H
#define FS_H

#include <stddef.h>
#include <stdint.h>

#include "types.h"

typedef uint16_t nlink_t;
typedef uint16_t mode_t;

typedef uint32_t ino_t;

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

struct fs_instance;

struct inode {
    nlink_t nlink;

    mode_t mode;

    ino_t ino;

    dev_t dev_on;

    uid_t uid;
    gid_t gid;

    union {
        struct {
            off_t firstblk;
            off_t size;
        };

        dev_t dev_type;
    };

    struct fs_instance *fs_on;
};

struct dentry {
    const char *name;
    struct inode *ino;
};

struct file;

struct fs_driver {
    struct fs_instance *(*mount)(struct file *, int, void *);
    void (*destroy)(struct fs_instance *);

    int (*create)(struct inode *, const char *, mode_t);
    struct dentry *(*lookup)(struct inode *, const char *);
    int (*readdir)(struct inode *, char **, size_t);
    int (*write)(struct inode *, off_t, const char *, size_t);
    int (*read)(struct inode *, off_t, char *, size_t);
};

struct fs_instance {
    struct fs_driver *driver;
    struct inode *root;
};

#endif
