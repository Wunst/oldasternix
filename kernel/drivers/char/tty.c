/*
 * file: kernel/drivers/char/tty.c
 *
 * Terminal device driver. Used for both virtual terminals (keyboard + monitor)
 * and physical ones (dedicated terminal hardware on a serial port).
 * see also: kernel/include/drivers/char/tty.h
 */

#include <errno.h>

#include <initcall.h>
#include <drivers/driver.h>
#include <drivers/major.h>
#include <drivers/tty.h>

#include "types.h"

/*
 * Minor device numbers:
 * 0    virtual console
 * 1    First serial port (if present)
 * ...  ...
 * 127  127th serial port (if present)
 * 128- Reserved (possibly more virtual consoles)
 */

int tty_read(dev_t dev, off_t pos, char *buf, size_t n)
{
    dev_t m = MINOR(dev);
    if (m == 0)
        return vconsole_read(buf, n);
    else if (m < 128)
        return serial_read(m, buf, n);
    return -ENODEV;
}

int tty_write(dev_t dev, off_t pos, const char *buf, size_t n)
{
    dev_t m = MINOR(dev);
    if (m == 0)
        return vconsole_write(buf, n);
    else if (m < 128)
        return serial_write(m, buf, n);
    return -ENODEV;
}

struct char_driver tty_driver = {
    .read = tty_read,
    .write = tty_write,
};

void register_tty_driver()
{
    register_char_driver(CHR_TTY, &tty_driver);
}

initcall(register_tty_driver);
