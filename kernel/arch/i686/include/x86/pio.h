#ifndef PIO_H
#define PIO_H

#include <stdint.h>

static void outb(uint16_t port, uint8_t val)
{
    asm volatile ("outb %%al, %%dx" :: "a"(val), "d"(port) : "memory");
}

static void outw(uint16_t port, uint16_t val)
{
    asm volatile ("outw %%ax, %%dx" :: "a"(val), "d"(port) : "memory");
}

static void outl(uint16_t port, uint32_t val)
{
    asm volatile ("outl %%eax, %%dx" :: "a"(val), "d"(port) : "memory");
}

static uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm ("inb %%dx, %%al" : "=a"(ret) : "d"(port) : "memory");
    return ret;
}

static uint16_t inw(uint16_t port)
{
    uint16_t ret;
    asm ("inw %%dx, %%ax" : "=a"(ret) : "d"(port) : "memory");
    return ret;
}

static uint32_t inl(uint16_t port)
{
    uint32_t ret;
    asm ("inl %%dx, %%eax" : "=a"(ret) : "d"(port) : "memory");
    return ret;
}

static void io_wait()
{
    /* Perform any IO operation on any unused port. (courtesy of OSDev wiki) */
    outb(0x80, 0);
}

#endif
