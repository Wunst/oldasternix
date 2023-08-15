ARCH?=i686
HOST=$(ARCH)-elf

CC=$(HOST)-gcc
AS=$(HOST)-as
AR=$(HOST)-ar

CCFLAGS?=-O2 -g -std=gnu99 -Wall -Wextra
LDFLAGS?=
ASFLAGS?=

DESTDIR?=$(shell pwd)/sysroot

PROJECTS=libc kernel

.PHONY: all install-all clean clean-install

all:
	for PROJECT in $(PROJECTS); \
	do cd $$PROJECT; \
	ARCH="$(ARCH)" CC="$(CC)" AS="$(AS)" AR="$(AR)" CCFLAGS="$(CCFLAGS)" LDFLAGS="$(LDFLAGS)" make; \
	cd ..; \
	done

install-all:
	for PROJECT in $(PROJECTS); \
	do cd $$PROJECT; \
	ARCH="$(ARCH)" CC="$(CC)" AS="$(AS)" AR="$(AR)" CCFLAGS="$(CCFLAGS)" LDFLAGS="$(LDFLAGS)" DESTDIR="$(DESTDIR)" make install; \
	cd ..; \
	done

clean:
	for PROJECT in $(PROJECTS); \
	do cd $$PROJECT; \
	make clean; \
	cd ..; \
	done

clean-install:
	rm -r $(DESTDIR)/*