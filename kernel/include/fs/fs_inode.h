#ifndef FS_INODE_H
#define FS_INODE_H

#include <fs/fs_types.h>

struct fs_instance;

struct inode {
    nlink_t nlink;

    mode_t mode;

    uid_t uid;
    gid_t gid;

    off_t firstblk;
    off_t size;

    struct fs_instance *fs_on;
};

#endif
