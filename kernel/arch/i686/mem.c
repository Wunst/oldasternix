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
#define PROT_PHYS_START 0x00100000
#define PROT_PHYS_END 0x00f00000

#define PAGE_LIMIT 0xffc00000

#define PAGE_TABLE_SIZE 1024

/* defined in linker.ld */
extern char __kernel_virtual_offset, __kernel_start, __kernel_end;

static uint32_t lower_bnd = PROT_PHYS_START, upper_bnd = PROT_PHYS_END;

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
    /* TODO: Check physical memory layout (ACPI or GRUB).
     * For now, we only use a single portion of memory (0x00100000-0x00efffff)
     * that is guaranteed to be free for use, which is going to waste a lot (in
     * fact, most) of physical memory space. */
    for (ret = lower_bnd; ret < upper_bnd; ret += PAGE_SIZE) {
        if (!is_phys_used(ret)) {
            set_phys_used(ret, true);
            return ret;
        }
    }
    return -1;
}

void mem_set_bounds(uint32_t lower, uint32_t upper)
{
    lower_bnd = lower;
    upper_bnd = upper;
}

void mem_set_used(uint32_t phys, uint32_t min_size)
{
    uint32_t page = phys & ~4095;
    do {
        set_phys_used(page, true);
        page += PAGE_SIZE;
    } while (page < phys + min_size);
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
