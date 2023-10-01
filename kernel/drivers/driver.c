#include <drivers/driver.h>

#include <errno.h>
#include <stdio.h>

#include <drivers/major.h>

struct char_driver *char_drivers[256];

void register_char_driver(unsigned char major, struct char_driver *dr)
{
    if (char_drivers[major]) {
        printf("err: device number taken: %d char\n"
                "\tdriver 1: %p\n"
                "\tdriver 2: %p\n", major, char_drivers[major], dr);
        return;
    }

    char_drivers[major] = dr;
}

int read_char(dev_t dev, off_t pos, char *buf, size_t n)
{
    if (!char_drivers[MAJOR(dev)])
        return -ENODEV;
    
    return char_drivers[MAJOR(dev)]->read(dev, pos, buf, n);
}

int write_char(dev_t dev, off_t pos, const char *buf, size_t n)
{
    if (!char_drivers[MAJOR(dev)])
        return -ENODEV;
    
    return char_drivers[MAJOR(dev)]->write(dev, pos, buf, n);
}
