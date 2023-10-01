#ifndef DRIVER_H
#define DRIVER_H

#include <stddef.h>

#include <fs/fs_types.h>

struct char_driver {
    int (*read)(dev_t, off_t, char *, size_t);
    int (*write)(dev_t, off_t, const char *, size_t);
};

void register_char_driver(unsigned char major, struct char_driver *dr);
int read_char(dev_t dev, off_t pos, char *buf, size_t n);
int write_char(dev_t dev, off_t pos, const char *buf, size_t n);

#endif
