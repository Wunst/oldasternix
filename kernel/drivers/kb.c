#include <drivers/kb.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <stdio.h>
#include <string.h>

#include <drivers/tty.h>

#define VC_KEY_QUEUE_LEN 256

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

static char input_queue[VC_KEY_QUEUE_LEN];
static size_t queue_idx = 0;

static uint8_t modifiers;

int vconsole_read(char *buf, size_t n)
{
    n = queue_idx < n ? queue_idx : n;

    /* Pass on `n` characters from queue. */
    memcpy(buf, input_queue, n);

    /* Move rest of queue forward. */
    memmove(input_queue, &input_queue[n], queue_idx - n);
    queue_idx -= n;

    return n;
}

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

        /* Ignore keys not mapped. */
        if (!ch)
            return;
        
        if (kb_get_mod(CTRL))
            /* Clear bits 7..5 -> send corresponding C0 control character */
            ch &= 0x1f;
        
        /*
         * Enqueue character. If queue is full, throw away very old inputs.
         * Those are probably void as no one has bothered to read them for 256
         * keystrokes.
         */
        if (queue_idx >= VC_KEY_QUEUE_LEN) {
            memmove(input_queue, &input_queue[1], queue_idx - 1);
            queue_idx--;
        }
        input_queue[queue_idx++] = ch;
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
