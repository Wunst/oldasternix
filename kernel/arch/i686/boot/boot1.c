#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <stdio.h>
#include <string.h>

#include <vendor/grub/multiboot2.h>

#include <drivers/major.h>
#include <drivers/tty.h>
#include <drivers/block/ramdisk.h>
#include <x86/interrupts.h>
#include <x86/mem.h>

#include "fs.h"

struct multiboot_info {
    uint32_t total_size;
    uint32_t _resv04;
    struct multiboot_tag tags[0];
};

/* defined in boot0.s */
extern void halt_loop(void);

extern struct fs_driver tmpfs_driver;

typedef void (*initcall_f)(void);

extern initcall_f __initcall_start, __initcall_end;

void hlinit(struct multiboot_info *mbi_phys)
{
    mem_init();
    tty_init();

    /*
     * Parse multiboot information structure.
     *
     * For now, only the memory map/info is useful for us.
     * TODO: Parse ACPI tables, reclaim ACPI memory, get framebuffer info
     */
    struct multiboot_tag *tag;

    for (tag = mbi_phys->tags;
        tag->type != MULTIBOOT_TAG_TYPE_END;
        tag = (void *)tag + ((tag->size + 7) & ~7)) {

        switch (tag->type) {
        case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
            mem_init_regions(
                ((struct multiboot_tag_basic_meminfo *)tag)->mem_lower * 1024,
                ((struct multiboot_tag_basic_meminfo *)tag)->mem_upper * 1024);
            break;
        
        case MULTIBOOT_TAG_TYPE_MMAP:
            for (struct multiboot_mmap_entry *mmap =
                    ((struct multiboot_tag_mmap *)tag)->entries;
                
                (void *)mmap < (void *)tag + tag->size;

                mmap = (void *)mmap +
                    ((struct multiboot_tag_mmap *)tag)->entry_size) {
                
                if (mmap->type != MULTIBOOT_MEMORY_AVAILABLE)
                    mem_set_used(mmap->addr, mmap->len);
            }
            break;
        
        case MULTIBOOT_TAG_TYPE_MODULE:
            ;
            /* Initial RAM disk is passed as a multiboot module. Map it
             * immediately after the kernel binary. */
            uint32_t start = ((struct multiboot_tag_module *)tag)->mod_start;
            uint32_t end = ((struct multiboot_tag_module *)tag)->mod_end;

            void *ramdisk = mem_map_range(K_MEM_START, start, end,
                DEFAULT_PAGE_FLAGS);
            
            dev_t rd = add_ramdisk(PAGE_SIZE, ramdisk, end - start);
            printf("info: added RAM disk as dev %d:%d\n", MAJOR(rd), MINOR(rd));

            break;
        }   
    }

    /* TODO: Kernel panic if no memory info */

    setup_interrupts();

    for (initcall_f *pp = &__initcall_start; pp < &__initcall_end; pp++)
        (**pp)();

    printf("Hello, world!\n");

    struct fs_instance *fs = tmpfs_driver.mount(NULL, 0, NULL);
    
    fs->driver->create(fs->root, "tty1", IT_CHR);
    
    struct dentry *de = fs->root->fs_on->driver->lookup(fs->root, "tty1");
    de->ino->dev_type = DEV(2, 1);
    de->ino->fs_on->driver->write(de->ino, 0, "\e[91;47mHello COM1", 19);

    fs->driver->create(fs->root, "console", IT_CHR);
    
    de = fs->root->fs_on->driver->lookup(fs->root, "console");
    de->ino->dev_type = DEV(2, 0);
    de->ino->fs_on->driver->write(de->ino, 0, "Hello from character device", 27);

    fs->driver->create(fs->root, "ram0", IT_BLK);

    char buf[4097];
    struct dentry *ram0 = fs->root->fs_on->driver->lookup(fs->root, "ram0");
    ram0->ino->dev_type = DEV(1, 0);
    ram0->ino->fs_on->driver->read(ram0->ino, 0, buf, 4096);
    buf[4096] = 0;
    printf("%s\n", buf);

    while (1) {
        char buf[10];
        int n;
        n = de->ino->fs_on->driver->read(de->ino, 0, buf, 10);
        de->ino->fs_on->driver->write(de->ino, 0, buf, n);
    }

    halt_loop();
}
