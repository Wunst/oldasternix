INCLUDES=-Iinclude -Iarch/$(ARCH)/include -I../libc/include
CCFLAGS+=$(INCLUDES) -D__is_kernel -ffreestanding

LIBS=-nostdlib -L../libc -lgcc -lk
LINKERSCRIPT=-Tarch/$(ARCH)/linker.ld
LDFLAGS+=$(LIBS) $(LINKERSCRIPT)

BIN=asternix.bin

SRCS=$(shell find -not -path 'arch/*' -and -regex '.*\.[cs]') $(shell find -path 'arch/$(ARCH)/*' -regex '.*\.[cs]')
OBJS=$(patsubst %,%.o,$(SRCS))

LIBK=../libc/libk.a

.PHONY: kernel install clean

kernel: $(BIN)

install:
	mkdir -p $(DESTDIR)/boot
	cp $(BIN) $(DESTDIR)/boot/$(BIN)

clean:
	rm $(BIN) $(shell find -name "*.o") $(shell find -name "*.d")

$(BIN): $(OBJS) $(LIBK)
	$(CC) $(OBJS) -o $@ $(CCFLAGS) $(LDFLAGS)

%.c.o: %.c
	$(CC) -c $< -o $@ -MD -MF $<.d -MT $@ $(CCFLAGS)

%.s.o: %.s
	$(AS) $< -o $@ -MD $<.d

-include $(OBJS:.o=.d)