#include <drivers/kb.h>

#include <stdint.h>

#include <stdio.h>

void kb_key_pressed(uint8_t keycode)
{
    printf("%02x\n", keycode);
}

void kb_key_released(uint8_t keycode)
{
    printf("%02x released\n", keycode);
}
