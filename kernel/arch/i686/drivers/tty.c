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

static void tty_scroll(int lines)
{
    int points = lines * VGA_WIDTH;
    int bytes = points * 2;
    memmove(vga_buffer, &vga_buffer[points], VGA_BUFFER_SIZE * 2 - bytes);
    memset(&vga_buffer[VGA_BUFFER_SIZE - points - 1], 0, bytes);
    pos -= points;
}

void tty_init()
{
    vga_buffer = mem_map_page(K_MEM_DEV_START, 0xb8000, DEFAULT_PAGE_FLAGS);
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
        int lines = (pos - VGA_BUFFER_SIZE) / VGA_WIDTH + 1;
        tty_scroll(lines);
    }
}

void tty_puts(const char *s)
{
    /* TODO: Make this more efficient? */
    while (*s)
        tty_putchar(*(s++));
}
