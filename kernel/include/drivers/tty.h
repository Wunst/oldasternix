#ifndef TTY_H
#define TTY_H

#include <stdint.h>

void tty_init(void);
void tty_putchar(char ch);
void tty_puts(const char *s);

#endif
