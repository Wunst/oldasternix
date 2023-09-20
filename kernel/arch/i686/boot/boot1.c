#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <stdio.h>
#include <string.h>

#include <vendor/grub/multiboot2.h>

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

    printf("Hello, world!\n");

    struct fs_instance *fs = tmpfs_driver.mount(NULL, 0, NULL);
    fs->driver->create(fs->root, "foo", IT_DIR);
    
    struct dentry *foo = fs->driver->lookup(fs->root, "foo");

    char buf[7];
    strcpy(buf, "bar000");
    for (int i = 0; i < 1000; i++) {
        buf[3] = '0' + i / 100;
        buf[4] = '0' + (i % 100) / 10;
        buf[5] = '0' + (i % 10);
        fs->driver->create(foo->ino, buf, IT_REG);
        struct dentry *file = fs->driver->lookup(foo->ino, buf);
        fs->driver->write(file->ino, 0, "Hello, ", 7);
        fs->driver->write(file->ino, 7, buf, 7);
    }

    for (int i = 0; i < 1000; i++) {
        buf[3] = '0' + i / 100;
        buf[4] = '0' + (i % 100) / 10;
        buf[5] = '0' + (i % 10);
        if (i == 999)
            printf("");
        struct dentry *file = fs->driver->lookup(foo->ino, buf);
        char new_buf[14];
        fs->driver->read(file->ino, 0, new_buf, 14);
        printf("%s : %s\n", buf, new_buf);
    }

    halt_loop();
}
