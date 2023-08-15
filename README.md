# \*nix (asternix) <sub>get it, cause the \* character is called an asterisk</sub>

## Goal

To create a well documented (as to be useful for educational purposes) clone of the original UNIX (POSIX-compatible where necessary).

This means that I do not make large-scale design decisions: I follow UNIX's for the better or worse, Keeping It Simple wherever possible.

## How to build

**If you're on Windows, you need to use either MINGW/MSYS/Cygwin or WSL2.**

Follow the [GCC Cross-Compiler](https://wiki.osdev.org/GCC_Cross-Compiler) tutorial on the OSDev wiki. You are going to need `i686-elf-gcc`, `i686-elf-as` `i686-elf-ld` and `i686-elf-ar` as well as GNU make.

Also install your distro's GRUB tools package if you want to create an ISO image, and QEMU, if you want to run it directly in an emulator.

Once you have your tools set up, run
```
make
make install-all
./grubiso.sh
```
This creates an ISO and boots the system in QEMU (if installed).

## Features

* boots on x86 legacy BIOS (32-bit protected mode) using GRUB
* can `printf` from the kernel
* (the humble beginnings of a) PS/2 keyboard driver

## Credits

Many thanks to (of course) the omniscient and omnibenevolent [OSDev wiki](https://wiki.osdev.org/) (and forum) without which we would still be living in caves.
