#include <drivers/tty.h>

#include <stddef.h>
#include <stdint.h>

#include <string.h>

#include <x86/mem.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_BUFFER_SIZE (VGA_WIDTH*VGA_HEIGHT)

#define TAB_SIZE 8

static uint8_t format = 0x07;
static uint16_t *vga_buffer;
static size_t pos = 0;

void tty_init()
{
    vga_buffer = mem_map_page(0xf0000000, 0xb8000, DEFAULT_PAGE_FLAGS);
}

void tty_putchar(char ch)
{
    if (ch == '\n') {
        pos = (pos / VGA_WIDTH + 1) * VGA_WIDTH;
    } else if (ch == '\t') {
        pos = (pos / TAB_SIZE + 1) * TAB_SIZE;
    } else {
        vga_buffer[pos] = (uint16_t)format << 8 | ch;
        pos++;
    }
    
    if (pos >= VGA_BUFFER_SIZE) {
        /* Scroll down */
        int amount = ((pos - VGA_BUFFER_SIZE) / VGA_WIDTH + 1) * VGA_WIDTH * 2;
        memmove(vga_buffer, &vga_buffer[amount], VGA_BUFFER_SIZE - amount);
        memset(&vga_buffer[VGA_BUFFER_SIZE - amount], 0, amount);
        pos -= amount;
    }
}

void tty_puts(const char *s)
{
    /* TODO: Make this more efficient? */
    while (*s)
        tty_putchar(*(s++));
}
