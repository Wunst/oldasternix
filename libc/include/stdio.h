/* SPDX-License-Identifier: GPL-3.0-or-later */
/**
 * @file libc/include/stdio.h
 * 
 * @brief C standard input/output
 * 
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/stdio.h.html
 */

#ifndef __STDIO_H
#define __STDIO_H

#include <stdarg.h>

#ifdef __is_kernel

/**
 * @brief Print one character
 * 
 * @arg ch Character (converted to `char`)
 * @returns ch
 * 
 * @section libk
 * 
 * Displays printed character on virtual console.
 */
int putchar(int ch);

/**
 * @brief Print string
 * 
 * @arg s Null-terminated string
 * @returns Number of characters printed
 * 
 * @section libk
 * 
 * Displays printed string on virtual console.
 */
int puts(const char *s);

/**
 * @brief Print format
 * 
 * @arg format Format string
 * @returns Number of character printed
 * 
 * @section libk
 * 
 * Displays printed string on virtual console (analogous to the operator
 * console on the original Unix systems).
 * 
 * @par Format specifiers
 * The following format specifiers are supported:
 *  Specifier   | Argument type  | Description
 * -------------|----------------|---------------------------------------------
 *  `%%`        | --             | Prints the '`%`' character
 *  `%%c`       | `char`         | Prints a character
 *  `%%s`       | `const char *` | Prints a string
 *  `%%d`/`%%i` | `int`          | Converts a signed integer to decimal
 *  `%%u`       | `unsigned`     | Converts an unsigned integer to decimal
 *  `%%x`/`%%X` | `unsigned`     | Converts an unsigned integer to hexadecimal
 *  `%%o`       | `unsigned`     | Converts an unsigned integer to hexadecimal
 *  `%%n`       | `int *`        | Returns number of characters written so far
 * 
 * The following modifiers are supported:
 * - `0`
 * - integer value for length
 * 
 * @see https://en.cppreference.com/w/cpp/io/c/fprintf
 * for all format specifiers defined by the standard
 */
int printf(const char *format, ...);

/**
 * @brief Print format from va_list
 * 
 * Like `printf()`, but accepts a `va_list` directly instead of varargs.
*/
int vprintf(const char *format, va_list args);

#endif

#endif
