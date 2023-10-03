#!/bin/sh
if [ ! -e sysroot/boot/asternix.bin ]
then
make all
make install-all
fi
mkdir -p sysroot/boot/grub
cp grub.cfg sysroot/boot/grub/grub.cfg
cp ramdisk sysroot/boot/ramdisk
grub-mkrescue sysroot -o asternix.iso
qemu-system-i386 -cdrom asternix.iso -s