#include <stddef.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <drivers/driver.h>

#include "fs.h"

#define TMPFS_BLK_SIZE 4096

extern struct fs_driver tmpfs_driver;

struct tmpfs_dentry {
    struct dentry base;
    struct tmpfs_dentry *next;
};

struct tmpfs_idir {
    struct inode base;
    struct tmpfs_dentry *first;
};

struct tmpfs_blk {
    char buf[TMPFS_BLK_SIZE];
    struct tmpfs_blk *nextblk;
};

struct tmpfs_ifile {
    struct inode base;
    struct tmpfs_blk firstblk;
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

static struct tmpfs_blk *find_blk(struct tmpfs_ifile *file, off_t pos)
{
    struct tmpfs_blk *ret = &file->firstblk;
    
    for (off_t o = TMPFS_BLK_SIZE; o <= pos; o += TMPFS_BLK_SIZE) {
        ret = ret->nextblk;
        if (!ret)
            break;
    }

    return ret;
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

struct dentry *tmpfs_lookup(struct inode *idir, const char *name)
{
    struct tmpfs_dentry *de;
    
    for (de = ((struct tmpfs_idir *)idir)->first; de != NULL; de = de->next) {
        if (strcmp(de->base.name, name) == 0)
            return &de->base;
    }

    return NULL;
}

int tmpfs_create(struct inode *idir, const char *name, mode_t mode)
{
    struct tmpfs_ifile *f;
    struct tmpfs_idir *d;
    struct inode *nod;

    if (tmpfs_lookup(idir, name))
        return -EEXIST;

    switch (mode & IT_TYPE) {
    case IT_REG:
        f = malloc(sizeof(struct tmpfs_ifile));

        f->base.nlink = 0;
        f->base.fs_on = idir->fs_on;
        f->base.mode = mode;
        f->base.uid = 0;
        f->base.gid = 0;
        f->base.firstblk = 0;
        f->base.size = 0;

        memset(f->firstblk.buf, 0, TMPFS_BLK_SIZE);
        f->firstblk.nextblk = NULL;

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
    
    case IT_CHR:
    case IT_BLK:
        nod = malloc(sizeof(struct inode));

        nod->nlink = 0;
        nod->fs_on = idir->fs_on;
        nod->mode = mode;
        nod->uid = 0;
        nod->gid = 0;
        nod->dev_type = 0;

        add_to_dir((struct tmpfs_idir *)idir, name, nod);

        return 0;

    default:
        return -EPERM;
    }
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
    if ((ifile->mode & IT_TYPE) == IT_DIR)
        return -EISDIR;
    
    if ((ifile->mode & IT_TYPE) == IT_CHR)
        return write_char(ifile->dev_type, pos, buf, n);
    
    if ((ifile->mode & IT_TYPE) == IT_BLK)
        return write_block(ifile->dev_type, pos, buf);
    
    if ((ifile->mode & IT_TYPE) != IT_REG)
        return -EPERM;
    
    if (n == 0)
        return 0;
    
    struct tmpfs_ifile *f = (struct tmpfs_ifile *)ifile;

    /* Find start block. Fails if `pos == size == capacity`. In this case... */
    struct tmpfs_blk *blk = find_blk(f, pos);
    if (!blk) {
        /* ...find block before that (last block)... */
        blk = find_blk(f, pos - 1);
        
        /* ...allocate a new one... */
        blk->nextblk = malloc(sizeof(struct tmpfs_blk));
        blk = blk->nextblk;
        
        /* ...and initialize it. */
        memset(blk->buf, 0, TMPFS_BLK_SIZE);
        blk->nextblk = NULL;
    }

    size_t end = pos + n;

    /* Write everything that fits in the first block. */
    size_t first_slice = TMPFS_BLK_SIZE - (pos % TMPFS_BLK_SIZE);
    first_slice = n < first_slice ? n : first_slice;
    memcpy(blk->buf + pos % TMPFS_BLK_SIZE, buf, first_slice);
    pos += first_slice;
    buf += first_slice;

    /* Now `pos % blksize == 0` and we can write block by block. */
    while (pos < end) {
        /* If no block, allocate one. */
        if (!blk->nextblk)
            blk->nextblk = malloc(sizeof(struct tmpfs_blk));
        
        blk = blk->nextblk;

        memcpy(blk->buf, buf, n < TMPFS_BLK_SIZE ? n : TMPFS_BLK_SIZE);
        pos += TMPFS_BLK_SIZE;
        buf += TMPFS_BLK_SIZE;
    }

    /* Update file size. */
    ifile->size = end < ifile->size ? ifile->size : end;
    
    return n;
}

int tmpfs_read(struct inode *ifile, off_t pos, char *buf, size_t n)
{
    if ((ifile->mode & IT_TYPE) == IT_DIR)
        return -EISDIR;
    
    if ((ifile->mode & IT_TYPE) == IT_CHR)
        return read_char(ifile->dev_type, pos, buf, n);
    
    if ((ifile->mode & IT_TYPE) == IT_BLK)
        return read_block(ifile->dev_type, pos, buf);
    
    if ((ifile->mode & IT_TYPE) != IT_REG)
        return -EPERM;

    if (pos + n >= ifile->size)
        n = ifile->size - pos;
    
    if (pos >= ifile->size || n == 0)
        return 0;
    
    struct tmpfs_ifile *f = (struct tmpfs_ifile *)ifile;
    
    struct tmpfs_blk *blk = find_blk(f, pos);
    if (!blk)
        return 0;
    
    size_t end = pos + n;

    /* Read everything from the first block: */
    size_t first_slice = TMPFS_BLK_SIZE - (pos % TMPFS_BLK_SIZE);
    first_slice = n < first_slice ? n : first_slice;
    memcpy(buf, blk->buf + pos % TMPFS_BLK_SIZE, first_slice);
    pos += first_slice;
    buf += first_slice;

    /* Now `pos % blksize == 0` and we can write block by block. */
    while (pos < end) {
        blk = blk->nextblk;
        memcpy(buf, blk->buf, n < TMPFS_BLK_SIZE ? n : TMPFS_BLK_SIZE);
        pos += TMPFS_BLK_SIZE;
        buf += TMPFS_BLK_SIZE;
    }

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
