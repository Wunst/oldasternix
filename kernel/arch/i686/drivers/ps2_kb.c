#include <drivers/ps2_kb.h>

#include <stddef.h>

#include <stdio.h>

#include <x86/interrupts.h>
#include <x86/pio.h>

#define IRQ1_KEYBOARD_DATA_READY 1
#define PS2_KB_DATA 0x60

INTERRUPT_HANDLER(ps2_key_pressed)()
{
    uint8_t scancode;
    scancode = inb(PS2_KB_DATA);

    if (scancode & 0x80) {
        printf("%02x released\n", scancode & ~0x80);
    } else {
        printf("%02x pressed\n", scancode);
    }

    pic_eoi(IRQ1_KEYBOARD_DATA_READY);
}

void ps2_kb_driversetup()
{
    set_isr(IRQ_OFFSET + IRQ1_KEYBOARD_DATA_READY, int_ps2_key_pressed);
    pic_clear_mask(IRQ1_KEYBOARD_DATA_READY);
}

void ps2_kb_drivercleanup()
{
    pic_set_mask(IRQ1_KEYBOARD_DATA_READY);
    set_isr(IRQ_OFFSET + IRQ1_KEYBOARD_DATA_READY, NULL);
}
