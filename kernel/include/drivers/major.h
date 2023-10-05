#ifndef MAJOR_H
#define MAJOR_H

#define DEV(major, minor) (((minor) << 8) | (major))
#define MAJOR(dev) ((unsigned char)(dev))
#define MINOR(dev) ((dev) >> 8)

#define NULL_DEV 0

#define CHR_MEMDEV 1
#define CHR_TTY 2

#define BLK_RAMDISK 1

#endif
