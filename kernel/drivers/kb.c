#include <drivers/kb.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <drivers/tty.h>

static char keycode_to_char[2][256] = {{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0,
    '`', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', 0,
    0, 0, 0, '/', '*', '-', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\\', 0,
    0, 0, '7', '8', '9', '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '\n', '4', '5',
    '6', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, 0, '1', '2', '3',
    '\n', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ' ', 0, 0, 0,
    0, 0, 0, 0, '0', '.'
}, {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0,
    '~', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', 0,
    0, 0, 0, '/', '*', '-', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '|', 0,
    0, 0, '7', '8', '9', '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '\n', '4', '5',
    '6', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, 0, '1', '2', '3',
    '\n', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ' ', 0, 0, 0,
    0, 0, 0, 0, '0', '.'
}};

static uint8_t modifiers;

void kb_key_pressed(uint8_t keycode)
{
    char ch;
    bool shift;
    switch (keycode) {
    case 0x80:
        kb_set_mod(LSHIFT, true);
        break;
    case 0x8b:
        kb_set_mod(RSHIFT, true);
        break;
    case 0xa0:
        kb_set_mod(LCTRL, true);
        break;
    case 0xa7:
        kb_set_mod(RCTRL, true);
        break;
    case 0xa2:
        kb_set_mod(LALT, true);
        break;
    case 0xa4:
        kb_set_mod(RALT, true);
        break;
    case 0x60:
        kb_set_mod(CAPS, !kb_get_mod(CAPS));
        break;
    default:
        shift = kb_get_mod(SHIFT) ^ kb_get_mod(CAPS);
        ch = keycode_to_char[shift][keycode];
        if (kb_get_mod(CTRL))
            /* Clear bits 7..5 -> send corresponding C0 control character */
            ch &= 0x1f;
        if (ch)
            tty_putchar(ch);
    }
}

void kb_key_released(uint8_t keycode)
{
    switch (keycode) {
    case 0x80:
        kb_set_mod(LSHIFT, false);
        break;
    case 0x8b:
        kb_set_mod(RSHIFT, false);
        break;
    case 0xa0:
        kb_set_mod(LCTRL, false);
        break;
    case 0xa7:
        kb_set_mod(RCTRL, false);
        break;
    case 0xa2:
        kb_set_mod(LALT, false);
        break;
    case 0xa4:
        kb_set_mod(RALT, false);
        break;
    }
}

bool kb_get_mod(enum kb_mod mod)
{
    return (modifiers & mod) != 0;
}

void kb_set_mod(enum kb_mod mod, bool value)
{
    if (value)
        modifiers |= mod;
    else
        modifiers &= ~mod;
}
