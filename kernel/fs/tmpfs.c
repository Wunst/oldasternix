#include <stddef.h>

#include <stdlib.h>
#include <string.h>
#include <fs/fs_dentry.h>
#include <fs/fs_driver.h>
#include <fs/fs_inode.h>
#include <fs/fs_types.h>

extern struct fs_driver tmpfs_driver;

struct tmpfs_dentry {
    struct dentry base;
    struct tmpfs_dentry *next;
};

struct tmpfs_idir {
    struct inode base;
    struct tmpfs_dentry *first;
};

struct tmpfs_ifile {
    struct inode base;
    char buf[1024];
};

static void add_to_dir(struct tmpfs_idir *dir, const char *name,
        struct inode *ino)
{
    char *name_buf = malloc(strlen(name));
    strcpy(name_buf, name);

    struct tmpfs_dentry *de = malloc(sizeof(struct tmpfs_dentry));

    de->base.name = name_buf;
    de->base.ino = ino;

    de->next = NULL;

    ino->nlink++;

    if (dir->first == NULL){
        dir->first = de;
    } else {
        struct tmpfs_dentry *last;
        for (last = dir->first; last->next != NULL; last = last->next)
            ;
        last->next = de;
    }
}

struct fs_instance *tmpfs_mount(struct file *file, int flags, void *args)
{
    struct fs_instance *fs = malloc(sizeof(struct fs_instance));

    fs->driver = &tmpfs_driver;

    struct tmpfs_idir *root = malloc(sizeof(struct tmpfs_idir));

    root->base.nlink = 0;
    root->base.fs_on = fs;
    root->base.mode = flags & I_PERMS;
    root->base.uid = 0;
    root->base.gid = 0;
    root->base.firstblk = 0;
    root->base.size = 0;

    root->first = NULL;
    fs->root = &root->base;

    return fs;
}

void tmpfs_destroy(struct fs_instance *fs)
{
    free(fs->root);
    free(fs);
}

int tmpfs_create(struct inode *idir, const char *name, mode_t mode)
{
    struct tmpfs_ifile *f;
    struct tmpfs_idir *d;

    switch (mode & IT_TYPE) {
    case IT_REG:
        f = malloc(sizeof(struct tmpfs_ifile));

        f->base.nlink = 0;
        f->base.fs_on = idir->fs_on;
        f->base.mode = mode;
        f->base.uid = 0;
        f->base.gid = 0;
        f->base.firstblk = 0;
        f->base.size = 1024;

        memset(f->buf, 0, 1024);

        add_to_dir((struct tmpfs_idir *)idir, name, &f->base);

        return 0;
    
    case IT_DIR:
        d = malloc(sizeof(struct tmpfs_idir));

        d->base.nlink = 0;
        d->base.fs_on = idir->fs_on;
        d->base.mode = mode;
        d->base.uid = 0;
        d->base.gid = 0;
        d->base.firstblk = 0;
        d->base.size = 0;

        d->first = NULL;

        add_to_dir(d, ".", &d->base);
        add_to_dir(d, "..", idir);
        add_to_dir((struct tmpfs_idir *)idir, name, &d->base);

        return 0;

    default:
        return -1;
    }
}

struct dentry *tmpfs_lookup(struct inode *idir, const char *name)
{
    struct tmpfs_dentry *de;
    
    for (de = ((struct tmpfs_idir *)idir)->first; de != NULL; de = de->next) {
        if (strcmp(de->base.name, name) == 0)
            return &de->base;
    }

    return NULL;
}

int tmpfs_readdir(struct inode *idir, char **names, size_t n)
{
    size_t count = 0;

    struct tmpfs_dentry *de;

    for (de = ((struct tmpfs_idir *)idir)->first; de != NULL; de = de->next) {
        if (count >= n)
            break;
        
        *(names++) = (char *)de->base.name;
        count++;
    }
    return count;
}

int tmpfs_write(struct inode *ifile, off_t pos, const char *buf, size_t n)
{
    if ((ifile->mode & IT_TYPE) != IT_REG)
        return -1;
    
    struct tmpfs_ifile *f = (struct tmpfs_ifile *)ifile;

    if (pos + n >= ifile->size)
        n = ifile->size - pos;
    
    memcpy(&f->buf[pos], buf, n);
    return n;
}

int tmpfs_read(struct inode *ifile, off_t pos, char *buf, size_t n)
{
    if ((ifile->mode & IT_TYPE) != IT_REG)
        return -1;
    
    struct tmpfs_ifile *f = (struct tmpfs_ifile *)ifile;

    if (pos + n >= ifile->size)
        n = ifile->size - pos;
    
    memcpy(buf, &f->buf[pos], n);
    return n;
}

struct fs_driver tmpfs_driver = {
    .mount = tmpfs_mount,
    .destroy = tmpfs_destroy,
    .create = tmpfs_create,
    .lookup = tmpfs_lookup,
    .readdir = tmpfs_readdir,
    .write = tmpfs_write,
    .read = tmpfs_read,
};
