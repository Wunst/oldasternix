#include <stdio.h>

#include <stdarg.h>

#include <string.h>

#ifdef __is_kernel

#include <drivers/tty.h>

static int putiradix(unsigned x, int r, char alpha, int min, char filler)
{
    char digits[11];
    int d, n = 0, i;
    do {
        d = x % r;
        if (d < 10) {
            digits[n] = '0' + d;
        } else {
            digits[n] = alpha + d - 10;
        }
        x /= r;
        n++;
    } while(x);
    for (i = 0; i < min - n; i++) {
        putchar(filler);
    }
    for (i = n - 1; i >= 0; i--) {
        putchar(digits[i]);
    }
    return min > n ? min : n;
}

int putchar(int ch)
{
    vconsole_write((char *) &ch, 1);
    return ch;
}

int puts(const char *s)
{
    return vconsole_write(s, strlen(s));
}

int printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    int ret = vprintf(format, args);

    va_end(args);

    return ret;
}

int vprintf(const char *format, va_list args)
{
    /* Potential format args */
    const char *s;
    char c;
    int d;
    int *p;

    int num_min;
    char num_filler;

    int n = 0;

    while (*format) {
        num_min = 0;
        num_filler = ' ';
        if (*format == '%') {
            format++;
            if (*format == '0') {
                num_filler = '0';
                format++;
            }
            while (*format >= '0' && *format <= '9') {
                num_min *= 10;
                num_min += *format - '0';
                format++;
            }
            switch (*format) {
            case '%':
                putchar('%');
                n++;
                break;
            case 's':
                s = va_arg(args, const char *);
                n += puts(s);
                break;
            case 'c':
                c = va_arg(args, int);
                putchar(c);
                n++;
                break;
            case 'd':
            case 'i':
                d = va_arg(args, int);
                if (d < 0) {
                    putchar('-');
                    n++;
                    d = -d;
                }
                n += putiradix(d, 10, 0, num_min, num_filler);
                break;
            case 'u':
                d = va_arg(args, unsigned);
                n += putiradix(d, 10, 0, num_min, num_filler);
                break;
            case 'p':
                n += puts("0x");
                /* fallthrough */
            case 'x':
                printf("");
                d = va_arg(args, unsigned);
                n += putiradix(d, 16, 'a', num_min, num_filler);
                break;
            case 'X':
                d = va_arg(args, unsigned);
                n += putiradix(d, 16, 'A', num_min, num_filler);
                break;
            case 'o':
                d = va_arg(args, unsigned);
                n += putiradix(d, 8, 0, num_min, num_filler);
                break;
            case 'n':
                p = va_arg(args, int *);
                *p = n;
                break;
            case '\0':
                putchar('%');
                n++;
                return n;
            default:
                putchar(*format);
            }
            format++;
        } else {
            putchar(*format);
            format++;
            n++;
        }
    }
    return n;
}

#endif
