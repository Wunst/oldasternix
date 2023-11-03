/* SPDX-License-Identifier: GPL-3.0-or-later */
/**
 * @file kernel/include/types.h
 * 
 * @brief Common types used across the kernel
 */
#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

/**
 * @brief Block or character device number
 * 
 * Consists of a major number (bits 0..7), identifying the driver and a minor
 * number (bits 8..31) which is handled by the driver and may be a partition or
 * port number or something else entirely.
 * 
 * Usually written as `major:minor`.
 * 
 * @note Block and character devices have seperate "namespaces", i.e. the same
 * major number can refer to a block and a character device.
 * 
 * @par Examples
 * - 1 = 1:0 block = First (minor 0) RAM disk (major 1)
 * - 257 = 1:1 block = Second RAM disk
 * - 257 = 1:1 char = `/dev/null`
 * - 2 = 2:0 char = First (minor 0) TTY (major 1) = virtual console
 * - 2 = 2:0 block = not assigned
 * 
 * @see kernel/include/drivers/major.h Major number definitions and utility macros
 */
typedef uint32_t dev_t;

/**
 * @brief User ID
 */
typedef uint32_t uid_t;

/**
 * @brief Group ID
 */
typedef uint32_t gid_t;

typedef uint64_t off_t;

typedef uint32_t blksize_t;

typedef uint32_t blkcnt_t;

#endif
