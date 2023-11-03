/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdlib.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __is_kernel

#include <stdio.h>

#include <x86/mem.h>

enum alloc_status {
    USED = (1 << 0),
    END_OF_MEMORY = (1 << 1),
};

/* TODO: Guarantee max alignment on changes to structure! */
struct alloc_header {
    uint8_t status;
    size_t size;
    
    struct alloc_header *prev_header;

    uint32_t _pad;
};

static struct alloc_header *first_hdr;

static struct alloc_header *find_next_hdr(struct alloc_header *hdr)
{
    return (void *)hdr + sizeof(struct alloc_header) + hdr->size;
}

void *malloc(size_t size)
{
    /* Guarantee max alignment: round up size to the nearest multiple of 16 */
    size = (size + 15) & ~15;

    /* Traverse headers until we have found a large enough space or reached the
     * end of allocated heap. */
    struct alloc_header *hdr = first_hdr;
    while (hdr && ((hdr->status & USED) || (hdr->size < size)) &&
            !(hdr->status & END_OF_MEMORY))
        hdr = find_next_hdr(hdr);
    
    /* Allocate more heap if necessary. */
    if (!hdr || (((hdr->status & USED) || (hdr->size < size)) &&
            (hdr->status & END_OF_MEMORY))) {
        size_t n = (size + sizeof(struct alloc_header)) / PAGE_SIZE + 1;

        struct alloc_header *new_hdr = mem_alloc(
                hdr ? (uint32_t)find_next_hdr(hdr) : K_MEM_HEAP_START,
                K_MEM_HEAP_END,
                n,
                DEFAULT_PAGE_FLAGS);
        
        new_hdr->status = END_OF_MEMORY;
        new_hdr->size = n * PAGE_SIZE - sizeof(struct alloc_header);
        new_hdr->prev_header = hdr;

        if (hdr)
            hdr->status &= ~END_OF_MEMORY;
        else
            first_hdr = new_hdr;

        hdr = new_hdr;
    }

    hdr->status |= USED;

    /* If we have much more space than required for the allocation, split the
     * block in two. */
    if (hdr->size >= size + 2 * sizeof(struct alloc_header)) {
        size_t remaining = hdr->size - (size + sizeof(struct alloc_header));

        hdr->size = size;

        struct alloc_header *next_hdr = find_next_hdr(hdr);
        next_hdr->status = 0;
        next_hdr->prev_header = hdr;
        next_hdr->size = remaining;

        if (hdr->status & END_OF_MEMORY) {
            hdr->status &= ~END_OF_MEMORY;
            next_hdr->status |= END_OF_MEMORY;
        } else {
            struct alloc_header *next_next = find_next_hdr(next_hdr);
            next_next->prev_header = next_hdr;
        }
    }

    return &hdr[1];
}

void free(void *ptr)
{
    struct alloc_header *hdr = ((struct alloc_header *)ptr) - 1;

    /* Merge with prev block if free (required to make one large allocation
     * possible in space previously fragmented by multiple smaller ones). */
    struct alloc_header *prev_hdr = hdr->prev_header;
    if (prev_hdr != NULL && !(prev_hdr->status & USED)) {
        prev_hdr->size += hdr->size;
        prev_hdr->status |= hdr->status & END_OF_MEMORY;
        hdr = prev_hdr;
    }
    
    /* Merge with next block if free. */
    if (!(hdr->status == END_OF_MEMORY)) {
        struct alloc_header *next_hdr = (void *)hdr +
                sizeof(struct alloc_header) + hdr->size;
        
        if (!(next_hdr->status & USED)) {
            hdr->size += next_hdr->size;
            hdr->status |= next_hdr->status & END_OF_MEMORY;

            if (!(next_hdr->status & END_OF_MEMORY)) {
                struct alloc_header *next_next = find_next_hdr(next_hdr);
                next_next->prev_header = hdr;
            }
        }
    }

    /* If we couldn't merge it, mark as free for later allocations/merges. */
    hdr->status &= ~USED;
}

#endif
