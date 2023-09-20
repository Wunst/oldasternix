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

static uint32_t alloc_phys()
{
    uint32_t ret;
    for (ret = 0; ret < 0xfffff000; ret += PAGE_SIZE) {
        if (!is_phys_used(ret)) {
            set_phys_used(ret, true);
            return ret;
        }
    }
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

void *mem_map_page(uint32_t virt_min, uint32_t phys, enum page_flags flags)
{
    if (phys % PAGE_SIZE != 0) {
        printf("err: mem: physical address not page aligned: 0x%x\n", phys);
        return NULL;
    }
    
    if (is_phys_used(phys)) {
        printf("warn: mem: physical memory already mapped: 0x%x\n", phys);
    }

    uint32_t virt;
    int pdi, pti;
    for (virt = virt_min; virt < PAGE_LIMIT; virt += PAGE_SIZE) {
        pdi = virt / HUGE_PAGE_SIZE;
        if ((page_directory[pdi] & PG_PRES) == 0) {
            page_directory[pdi] = alloc_phys() | PAGE_DIRECTORY_FLAGS;
            memset(&page_tables[pdi * PAGE_TABLE_SIZE], 0, PAGE_SIZE);
        }

        pti = virt / PAGE_SIZE;
        if ((page_tables[pti] & PG_PRES) == 0) {
            page_tables[pti] = phys | flags;
            return (void *)virt;
        }
    }

    return NULL;
}

void *mem_alloc(uint32_t virt, uint32_t virt_end_max, size_t n,
        enum page_flags flags)
{
    /* TODO: DRY this with `mem_map_page()` */

    /* TODO: Optimize checks if page used? */
    for (uint32_t single = virt; single < virt + n; single += PAGE_SIZE) {
        if (single > virt_end_max)
            return NULL;
        
        if ((page_directory[single / HUGE_PAGE_SIZE] & PG_PRES) == 0)
            continue;
        
        if ((page_tables[single / PAGE_SIZE] & PG_PRES))
            return NULL;
    }

    /* No used page within reach - we've found space! */
    for (uint32_t single = virt; single < virt + n; single += PAGE_SIZE) {
        int pdi = single / HUGE_PAGE_SIZE;
        if ((page_directory[pdi] & PG_PRES) == 0) {
            page_directory[pdi] = alloc_phys() | PAGE_DIRECTORY_FLAGS;
            memset(&page_tables[pdi * PAGE_TABLE_SIZE], 0, PAGE_SIZE);
        }

        int pti = single / PAGE_SIZE;
        if ((page_tables[pti] & PG_PRES) == 0) {
            page_tables[pti] = alloc_phys() | flags;
        }
    }
    return (void *)virt;
}
