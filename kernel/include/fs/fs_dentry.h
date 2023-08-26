#ifndef FS_DENTRY_H
#define FS_DENTRY_H

struct inode;

struct dentry {
    const char *name;
    struct inode *ino;
};

#endif
