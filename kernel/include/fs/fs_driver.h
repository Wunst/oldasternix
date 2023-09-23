#ifndef FS_DRIVER
#define FS_DRIVER

#include <stddef.h>

#include <fs/fs_types.h>

struct dentry;
struct file;
struct inode;

struct fs_instance;

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
