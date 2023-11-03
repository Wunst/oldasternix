/* SPDX-License-Identifier: GPL-3.0-or-later */
/**
 * @file libc/include/errno.h
 *
 * @brief POSIX system error numbers
 * 
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/errno.h.html
 */

#ifndef __ERRNO_H
#define __ERRNO_H

/* Reserved. Used generically by file system code before this header was
 * implemented, may still be in use somewhere so we don't reassign it. */
#define _ELEGACY 1

/**
 * @brief Permission denied.
 */
#define EACCES 2

/**
 * @brief File exists.
 */
#define EEXIST 3

/**
 * @brief File too large.
 * e.g. writing to a position past the maximum allowed offset (file size)
 */
#define EFBIG 4

/**
 * @brief Operation not permitted.
 * e.g. creating a symlink or device file on a FAT file system
 */
#define EPERM 5

/**
 * @brief Bad address.
 * e.g. a process tries to `read()` into a buffer owned by another process.
 * Because `read()` is a syscall, it runs in kernel mode and this wouldn't be
 * prevented by x86 memory protection, so the kernel needs to detect this sort
 * of potentially malicious behaviour.
 */
#define EFAULT 6

/**
 * @brief No space left on device.
 */
#define ENOSPC 7

/**
 * @brief Is a directory.
 */
#define EISDIR 8

/**
 * @brief Not a directory or symbolic link to a directory.
 */
#define ENOTDIR 9

/**
 * @brief No such device.
 */
#define ENODEV 10

#endif
