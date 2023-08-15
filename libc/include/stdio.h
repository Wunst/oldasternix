#ifndef __STDIO_H
#define __STDIO_H

#ifdef __is_kernel

int putchar(int ch);
int puts(const char *s);
int printf(const char *format, ...);

#endif

#endif
