/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef __STRING_H
#define __STRING_H

/* NULL, size_t */
#include <stddef.h>

/* String manipulation */

char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, size_t n);

/* String examination */

size_t strlen(const char *s);
int strcmp(const char *lhs, const char *rhs);
int strncmp(const char *lhs, const char *rhs, size_t n);
char *strchr(const char *s, int ch);
char *strrchr(const char *s, int ch);
size_t strspn(const char *dest, const char *src);
size_t strcspn(const char *dest, const char *src);
char *strpbrk(const char *dest, const char *breakset);
char *strstr(const char *dest, const char *src);
/* char *strtok(char *s, const char *delimset); */

/* Character array manipulation */

void *memchr(const void *ptr, int ch, size_t n);
int memcmp(const void *lhs, const void *rhs, size_t n);
void *memset(void *dest, int ch, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);

#endif
