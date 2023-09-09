#include <stddef.h>
#include <stdint.h>

#include <stdio.h>

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
            mem_set_bounds(
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

    setup_interrupts();

    ps2_kb_driversetup();

    printf("Hello, world!\n");

    struct fs_instance *fs = tmpfs_driver.mount(NULL, 0, NULL);
    fs->driver->create(fs->root, "foo", IT_DIR);
    
    struct dentry *foo = fs->driver->lookup(fs->root, "foo");
    fs->driver->create(foo->ino, "bar", IT_REG);
    fs->driver->create(foo->ino, "baz", IT_REG);

    char *names[2];
    fs->driver->readdir(foo->ino, names, 2);
    printf("%s\n", names[0]);
    printf("%s\n", names[1]);

    struct dentry *baz = fs->driver->lookup(foo->ino, "baz");
    fs->driver->write(baz->ino, 0, "Hello, files!\n", 15);
    
    char buf[15];
    fs->driver->read(baz->ino, 0, buf, 15);

    printf("%s\n", buf);

    halt_loop();
}
