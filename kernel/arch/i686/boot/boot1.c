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

    fs->driver->create(fs->root, "lorem.txt", IT_REG);

    const char *lorem = "Lorem ipsum dolor sit amet, consectetur adipiscing "
            "elit. In fermentum ac magna at varius. Vestibulum tortor tellus, "
            "cursus id massa sit amet, placerat ullamcorper risus. Nullam ac "
            "neque vel nibh pellentesque dapibus. Donec ac eros id eros "
            "faucibus tincidunt. Fusce faucibus dapibus tincidunt. Mauris "
            "mollis libero id magna ultricies vestibulum. Donec cursus ante "
            "urna, ac tristique metus aliquam mattis. Praesent ut tempus sem, "
            "eget lobortis felis. Praesent sed nibh ipsum. Pellentesque vitae "
            "ante sem. Ut lobortis ex purus, in varius nunc ultricies eget.";

    struct dentry *de = fs->driver->lookup(fs->root, "lorem.txt");
    de->ino->fs_on->driver->write(de->ino, 0, "0001", 4);
    de->ino->fs_on->driver->write(de->ino, 3, "23", 2);
    if (de->ino->fs_on->driver->write(de->ino, 6, "0001", 2) > 0)
        printf("ERROR");
    de->ino->fs_on->driver->write(de->ino, 5, lorem, 556);
    de->ino->fs_on->driver->write(de->ino, 560, "0002", 4);
    de->ino->fs_on->driver->write(de->ino, 564, lorem, 556);
    de->ino->fs_on->driver->write(de->ino, 1119, "0003", 4);
    de->ino->fs_on->driver->write(de->ino, 1123, lorem, 4);
    de->ino->fs_on->driver->write(de->ino, 1678, "0004", 4);
    de->ino->fs_on->driver->write(de->ino, 1683, lorem, 556);
    de->ino->fs_on->driver->write(de->ino, 2234, lorem, 556);
    de->ino->fs_on->driver->write(de->ino, 2789, lorem, 556);
    de->ino->fs_on->driver->write(de->ino, 3344, lorem, 556);
    de->ino->fs_on->driver->write(de->ino, 3899, "0005", 4);
    de->ino->fs_on->driver->write(de->ino, 3903, lorem, 556);

    char buf[4200];
    de->ino->fs_on->driver->read(de->ino, 0, buf, 4200);
    printf("%s\n", buf);

    halt_loop();
}
