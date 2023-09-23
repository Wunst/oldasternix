#ifndef MEM_H
#define MEM_H

#include <stddef.h>
#include <stdint.h>

#define PAGE_SIZE 4096
#define HUGE_PAGE_SIZE 4194304

/*
 * Memory map
 *
 * 0x00000000: NULL (invalid, never mapped)
 * 
 * 0x00100000: User binary loaded here, followed immediately by user heap.
 * This is in the spirit of the original UNIX systems where the "heap" was just
 * the "data segment" of the binary dynamically extended with `sbrk()`.
 * 
 * 0xbfffffff: User stack starts here and grows downwards.
 * NOTE: The kernel allocates pages for the stack automatically on page fault.
 * It needs to check if heap and stack are going to collide!
 * 
 * Max size of user memory (binary + heap + stack) = 3GiB
 * 
 * 0xc0000000: Start of physical memory. BIOS and bootloader memory.
 * 0xc0100000: Kernel binary loaded here
 * Max size of kernel binary = 256MiB (sufficient! Linux + initrd ~ 110MiB)
 * 
 * 0xd0000000-0xefffffff: Kernel heap
 * Max size of kernel heap = 512MiB (sufficient?)
 * 
 * 0xf0000000-0xffbfffff: Memory-mapped devices
 * 
 * 0xffc00000: Page tables (see mem.c for how this works)
 * 
 * 0xfffff000: Page directory
 */
#define U_MEM_START 0x00100000
#define U_MEM_END 0xbfffffff
#define K_MEM_START 0xc0000000
#define K_MEM_HEAP_START 0xd0000000
#define K_MEM_HEAP_END 0xefffffff
#define K_MEM_DEV_START 0xf0000000

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

/*
 * Tells the memory manager the basic memory regions. (x86 specific)
 * `lower`: size of lower memory (<1MiB, starts at 0)
 * `upper`: size of upper memory (>1MiB, starts at 1MiB)
 * e.g. on a system with video memory starting at 0xa0000 GRUB does not use
 * the memory map for this but sets the `basic_meminfo#lower` field to 0xa0000.
 * 
 * NEEDS to be called before any physical memory allocations happen.
 */
void mem_init_regions(uint32_t lower, uint32_t upper);

void mem_set_used(uint64_t phys, uint64_t min_size);
void mem_init(void);

/*
 * Maps a physical page starting at `phys` at the NEXT FREE virtual page,
 * searching from `virt_min`.
 */
void *mem_map_page(uint32_t virt_min, uint32_t phys, enum page_flags flags);

/*
 * Allocates and maps `n` physical pages starting at `virt`. Returns a pointer
 * to `virt` if successful, or NULL if memory is already taken, or the memory
 * region would reach beyond `virt_end_max`, or `virt` isn't page aligned.
 */
void *mem_alloc(uint32_t virt, uint32_t virt_end_max, size_t n,
        enum page_flags flags);

#endif
