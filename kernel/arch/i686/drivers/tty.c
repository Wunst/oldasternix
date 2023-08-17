#include <drivers/tty.h>

#include <stdbool.h>
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

static bool processing_sequence;

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
    vga_buffer = mem_map_page(0xf0000000, 0xb8000, DEFAULT_PAGE_FLAGS);
}

void tty_putchar(char ch)
{
    switch (ch) {
    case '\n': /* Newline */
    case '\r': /* Carriage Return (^M) */
        pos = (pos / VGA_WIDTH + 1) * VGA_WIDTH;
        break;

    case '\t': /* Tab */
        pos = (pos / TAB_SIZE + 1) * TAB_SIZE;
        break;
    
    case '\f': /* Form Feed (^L, Clear Screen) */
        memset(vga_buffer, 0, VGA_BUFFER_SIZE * 2);
        pos = 0;
        break;
    
    case '\e': /* Escape (^[, Begin ANSI escape sequence) */
        processing_sequence = true;
        break;

    default:
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
