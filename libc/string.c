/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <string.h>

static char *seek_end(char *s)
{
    while (*s)
        s++;
    return s;
}

char *strcpy(char *dest, const char *src)
{
    char *ret = dest;
    while ((*(dest++) = *(src++)))
        ;
    return ret;
}

char *strncpy(char *dest, const char *src, size_t n)
{
    char *ret = dest;
    while (n-- && (*(dest++) = *(src++)))
        ;
    *dest = 0;
    return ret;
}

char *strcat(char *dest, const char *src)
{
    char *ret = dest;
    dest = seek_end(dest);
    strcpy(dest, src);
    return ret;
}

char *strncat(char *dest, const char *src, size_t n)
{
    char *ret = dest;
    dest = seek_end(dest);
    strncpy(dest, src, n);
    return ret;
}

size_t strlen(const char *s)
{
    size_t n = 0;
    while (*(s++))
        n++;
    return n;
}

int strcmp(const char *lhs, const char *rhs)
{
    char l, r;
    do {
        l = *(lhs++);
        r = *(rhs++);
    } while (l && r && l == r);
    return r - l;
}

int strncmp(const char *lhs, const char *rhs, size_t n)
{
    char l, r;
    if (!n)
        return 0;
    do {
        l = *(lhs++);
        r = *(rhs++);
    } while (--n && l && r && l == r);
    return r - l;
}

char *strchr(const char *s, int ch)
{
    do {
        if (*s == ch)
            return (char *)s;
    } while(*(s++));
    return NULL;
}

char *strrchr(const char *s, int ch)
{
    s = seek_end((char *)s);
    do {
        if (*s == ch)
            return (char *)s;
    } while(*(s--));
    return NULL;
}

size_t strspn(const char *dest, const char *src)
{
    size_t n = 0;
    while (*dest && strchr(src, *(dest++)))
        n++;
    return n;
}

size_t strcspn(const char *dest, const char *src)
{
    size_t n = 0;
    while (*dest && !strchr(src, *(dest++)))
        n++;
    return n;
}

char *strpbrk(const char *dest, const char *breakset)
{
    dest += strcspn(dest, breakset);
    if (*dest)
        return (char *)dest;
    return NULL;
}

char *strstr(const char *dest, const char *src)
{
    size_t n = strlen(src);
    while (*dest) {
        if (strncmp(dest, src, n) == 0)
            return (char *)dest;
        dest++;
    }
    return NULL;
}

void *memchr(const void *ptr, int ch, size_t n)
{
    while(n--) {
        if (*(unsigned char *)ptr == ch)
            return (void *)ptr;
    }
    return NULL;
}

int memcmp(const void *lhs, const void *rhs, size_t n)
{
    unsigned char l, r;
    if (!n)
        return 0;
    do {
        l = *(unsigned char *)(lhs++);
        r = *(unsigned char *)(rhs++);
    } while (--n && l == r);
    return r - l;
}

void *memset(void *dest, int ch, size_t n)
{
    void *ret = dest;
    while (n--) {
        *(unsigned char *)(dest++) = (unsigned char)ch;
    }
    return ret;
}

void *memcpy(void *dest, const void *src, size_t n)
{
    void *ret = dest;
    while (n--) {
        *(unsigned char *)(dest++) = *(unsigned char *)(src++);
    }
    return ret;
}

void *memmove(void *dest, const void *src, size_t n)
{
    void *ret = dest;
    if (dest < src) {
        while (n--) {
            *(unsigned char *)(dest++) = *(unsigned char *)(src++);
        }
    }
    if (dest > src) {
        src += n;
        dest += n;
        while (n--) {
            *(unsigned char *)(--dest) = *(unsigned char *)(--src);
        }
    }
    return ret;
}
