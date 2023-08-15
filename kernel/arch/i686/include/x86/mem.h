/*
 * file: mem.h
 * description: Kernel memory management functionality. This is very
 *  architecture specific and its implementation is not portable at all at this
 *  point.
 */

#ifndef MEM_H
#define MEM_H

#include <stdint.h>

enum page_flags {
    PG_PRES = 1,
    PG_RW = 2,
    PG_US = 4,
    PG_PWT = 8,
    PG_NCACHE = 16,
    PG_ACCESSED = 32,
    PG_DIRTY = 64,

    /* Only on PTE. */
    PG_PAT = 128,

    /* Only on PDE. Completely unsupported by the allocator right now!! */
    PG_HUGE = 128,
    PG_HUGE_PAT = 4096,

    DEFAULT_PAGE_FLAGS = PG_PRES | PG_RW,
    PAGE_DIRECTORY_FLAGS = PG_PRES | PG_RW | PG_US,
};

void mem_init(void);
void *mem_map_page(uint32_t virt_min, uint32_t phys, enum page_flags flags);

#endif
