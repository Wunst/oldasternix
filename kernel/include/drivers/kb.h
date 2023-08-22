#ifndef KB_H
#define KB_H

#include <stdbool.h>
#include <stdint.h>

enum kb_mod {
    LSHIFT = 1,
    RSHIFT = 2,
    SHIFT = LSHIFT | RSHIFT,
    LCTRL = 4,
    RCTRL = 8,
    CTRL = LCTRL | RCTRL,
    LALT = 16,
    RALT = 32,
    ALT = LALT | RALT,
    CAPS = 64,
};

void kb_key_pressed(uint8_t keycode);
void kb_key_released(uint8_t keycode);

bool kb_get_mod(enum kb_mod mod);
void kb_set_mod(enum kb_mod mod, bool value);

#endif
