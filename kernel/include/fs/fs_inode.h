#ifndef FS_INODE_H
#define FS_INODE_H

#include <fs/fs_types.h>

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

#endif
