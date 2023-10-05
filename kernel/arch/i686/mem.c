/*
 * TODO:
 *  - support huge pages
 */

#include <x86/mem.h>

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <string.h>
#include <stdio.h>

/* The portion of physical memory that is guaranteed to be usable */
/* TODO: Get rid of this. Are systems even required to have high memory? */
#define PROT_PHYS_START 0x00100000
#define PROT_PHYS_END 0x00f00000

#define PAGE_LIMIT 0xffc00000

#define PAGE_TABLE_SIZE 1024

/* defined in linker.ld */
extern char __kernel_virtual_offset, __kernel_start, __kernel_end;

static uint32_t lower_bound = PROT_PHYS_START, upper_bound = PROT_PHYS_END;

/* By mapping the last PDE to the page directory itself, we can access all
 * paging structures (in the current address space) starting at 0xffc00000,
 * through the CPU interpreting the page directory as a page table and
 * therefore the page tables it points to as its pages. */
static uint32_t *page_directory = (uint32_t *)0xfffff000;

static uint32_t *page_tables = (uint32_t *)0xffc00000;

/* One bit in this structure marks one page of physical memory as used. */
static uint8_t phys_page_bitmap[INT32_MAX / PAGE_SIZE / 8];

static bool is_phys_used(uint32_t phys)
{
    int byte = phys / PAGE_SIZE / 8, bit = phys / PAGE_SIZE % 8;
    return (phys_page_bitmap[byte] >> bit) & 1;
}

static void set_phys_used(uint32_t phys, bool used)
{
    int byte = phys / PAGE_SIZE / 8, bit = phys / PAGE_SIZE % 8;
    if (used) {
        phys_page_bitmap[byte] |= 1 << bit;
    } else {
        phys_page_bitmap[byte] &= ~(1 << bit);
    }
}

static bool is_virt_region_used(size_t *start_page, size_t n)
{
    size_t end_page = *start_page + n;

    if (end_page < *start_page) {
        /* Out of memory. */
        *start_page = 0;
        return true;
    }

    for (size_t page = *start_page; page < end_page; page++) {
        if ((page_directory[page / 1024] & PG_PRES) == 0) {
            /* Skip rest of this huge page as it can't be mapped, and continue. */
            page |= (HUGE_PAGE_SIZE - 1) & ~4095;
            continue;
        }
        
        if ((page_tables[page] & PG_PRES)) {
            /* Return next page to try for finding a free region. */
            *start_page = page + 1;
            return true;
        }
    }

    return false;
}

static uint32_t alloc_phys()
{
    uint32_t ret;
    for (ret = lower_bound; ret < upper_bound; ret += PAGE_SIZE) {
        if (!is_phys_used(ret)) {
            set_phys_used(ret, true);
            return ret;
        }
    }
    printf("err: mem: out of memory (phys)\n");
    return -1;
}

void mem_init_regions(uint32_t lower, uint32_t upper)
{
    /* Mark memory between the end of lower memory and start of upper memory as
     * used (e.g. video memory). */
    for (uint32_t page = lower & ~4095; page < 0x100000; page += PAGE_SIZE)
        set_phys_used(page, true);
    
    /* Set the upper bound to the end of upper memory. */
    upper_bound = 0x100000 + upper;

    /* Set the lower bound to lower memory. It is safe to use now. */
    lower_bound = 0;
}

void mem_set_used(uint64_t phys, uint64_t min_size)
{
    /* Memory map is 64-bit, ignore everything outside of our 32-bit address
     * space. */
    if (phys > UINT32_MAX)
        return;
    
    /* Cut off larger regions to 32-bit. */
    uint64_t max = phys + min_size;
    max = (max > UINT32_MAX) ? UINT32_MAX : max;
    
    uint32_t page = phys & ~4095;
    do {
        set_phys_used(page, true);
        page += PAGE_SIZE;
    } while (page < max && page != 0);
}

void mem_init()
{
    uint32_t phys;
    for (phys = (uint32_t)&__kernel_start; phys < (uint32_t)&__kernel_end;
            phys += PAGE_SIZE) {
        set_phys_used(phys - (uint32_t)&__kernel_virtual_offset, 1);
    }
}

void *mem_map(uint32_t virt_min, size_t n, uint32_t phys,
        enum page_flags flags)
{
    if (phys % PAGE_SIZE != 0) {
        printf("err: mem: physical address not page aligned: 0x%x\n", phys);
        return NULL;
    }

    size_t page_start = virt_min / PAGE_SIZE;

    /* Find a free page. */
    while (is_virt_region_used(&page_start, n)) {
        if (page_start == 0) {
            printf("err: mem: out of memory (virt)\n");
            return NULL;
        }
    }

    size_t page_end = page_start + n;

    for (size_t page = page_start; page < page_end; page++) {
        if ((page_directory[page / 1024] & PG_PRES) == 0) {
            page_directory[page / 1024] = alloc_phys() | PAGE_DIRECTORY_FLAGS;
            memset(&page_tables[page], 0, PAGE_SIZE);
        }

        page_tables[page] = phys | flags;
        phys += PAGE_SIZE;
    }

    return (void *)((uint32_t)page_start * PAGE_SIZE);
}

void *mem_map_range(uint32_t virt_min, uint32_t phys_start, uint32_t phys_end,
        enum page_flags flags)
{
    if (phys_end < phys_start) {
        printf("err: mem: invalid region: %08X..%08X", phys_start, phys_end);
        return NULL;
    }

    size_t page_offset = phys_start & 4095;
    size_t n = (phys_end - phys_start + 4095) / PAGE_SIZE;

    phys_start &= ~4096;

    return mem_map(virt_min, n, phys_start, flags) + page_offset;
}

void *mem_alloc(uint32_t virt, uint32_t virt_end_max, size_t n,
        enum page_flags flags)
{
    size_t page_start = virt / PAGE_SIZE;
    size_t page_max = virt_end_max / PAGE_SIZE;

    if (page_start + n > page_max)
        return NULL;
    
    if (is_virt_region_used(&page_start, n))
        return NULL;

    /* No used page within reach - we've found space! */
    for (size_t page = page_start; page < page_start + n; page++)
        mem_map(page * PAGE_SIZE, 1, alloc_phys(), flags);

    return (void *)virt;
}
