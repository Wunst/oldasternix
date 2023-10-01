/*
 * file: kernel/include/drivers/tty.h
 *
 * Common interface for virtual and serial terminals and across platforms.
 * Platform code (e.g. kernel/arch/i686/drivers/tty.c) supplies methods for the
 * low-level details such as writing to ports, a common driver
 * (kernel/drivers/char/tty.c) wraps them into a character device.
 */

#include <stddef.h>

void tty_init(void);

int vconsole_read(char *buf, size_t n);
int vconsole_write(const char *buf, size_t n);

int serial_read(int port, char *buf, size_t n);
int serial_write(int port, const char *buf, size_t n);
