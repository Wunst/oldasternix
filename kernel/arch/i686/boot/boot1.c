#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <stdio.h>
#include <string.h>

#include <vendor/grub/multiboot2.h>

#include <drivers/major.h>
#include <drivers/tty.h>
#include <drivers/ps2_kb.h>
#include <fs/fs_dentry.h>
#include <fs/fs_driver.h>
#include <fs/fs_inode.h>
#include <x86/interrupts.h>
#include <x86/mem.h>

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
                ((struct multiboot_tag_basic_meminfo *)tag)->mem_lower,
                ((struct multiboot_tag_basic_meminfo *)tag)->mem_upper);
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
        }   
    }

    /* TODO: Kernel panic if no memory info */

    setup_interrupts();

    ps2_kb_driversetup();

    for (initcall_f *pp = &__initcall_start; pp < &__initcall_end; pp++)
        (**pp)();

    printf("Hello, world!\n");

    struct fs_instance *fs = tmpfs_driver.mount(NULL, 0, NULL);

    fs->driver->create(fs->root, "urandom", IT_CHR);
    
    struct dentry *de = fs->root->fs_on->driver->lookup(fs->root, "urandom");
    de->ino->dev_type = DEV(1, 6);

    char random[10];
    de->ino->fs_on->driver->read(de->ino, 0, random, 10);
    
    for (int i = 0; i < 10; i++)
        printf("%d\n", random[i]);

    halt_loop();
}
