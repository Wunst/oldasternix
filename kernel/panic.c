#include "panic.h"

#include <stdarg.h>

#include <stdio.h>

extern void halt_loop(void);

void panic(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    vprintf(format, args);
    
    va_end(args);

    void *unwind_ptr;
    while (panic_unwind(&unwind_ptr))
        printf("\tat %p\n", unwind_ptr);

    halt_loop();
}

int panic_unwind(void **unwind_ptr)
{
    /* TODO */
    return 0;
}
