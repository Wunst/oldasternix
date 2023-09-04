#include <stdlib.h>

#include <stddef.h>
#include <stdint.h>

#ifdef __is_kernel

#include <stdio.h>

void *malloc(size_t size)
{
    static uint32_t addr = __BIGGEST_ALIGNMENT__;
    
    void *ret = (void *)addr;

    addr += size;
    if (addr % __BIGGEST_ALIGNMENT__ != 0)
        addr += __BIGGEST_ALIGNMENT__ - addr % __BIGGEST_ALIGNMENT__;
    
    return ret;
}

void free(void *ptr)
{
}

#endif
