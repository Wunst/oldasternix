INCLUDES=-Iinclude
CCFLAGS+=$(INCLUDES) -ffreestanding

LIBK_INCLUDES=-I../kernel/include -I../kernel/arch/$(ARCH)/include
LIBK_CCFLAGS=$(LIBK_INCLUDES) -D__is_kernel

LIBS=-nostdlib -lgcc
LDFLAGS+=$(LIBS)

LIBC=libc.so
LIBK=libk.a

SRCS=$(shell find -not -path 'arch/*' -and -regex '.*\.[cs]') $(shell find -path 'arch/$(ARCH)/*' -regex '.*\.[cs]')
OBJS=$(patsubst %,%.o,$(SRCS))

LIBK_OBJS=$(patsubst %,%.libk.o,$(SRCS))

.PHONY: all libc libk install clean

all: libc libk

libc: $(LIBC)

libk: $(LIBK)

install:
	mkdir -p $(DESTDIR)/lib
	cp $(LIBC) $(DESTDIR)/lib/$(LIBC)

clean:
	rm $(LIBC) $(LIBK) $(shell find -name "*.o") $(shell find -name "*.d")

$(LIBC): $(OBJS)
	$(CC) $^ -o $@ $(CCFLAGS) $(LDFLAGS)

$(LIBK): $(LIBK_OBJS)
	$(AR) cr $@ $^

%.c.o: %.c
	$(CC) -c $< -o $@ -MD -MF $<.d -MT $@ $(CCFLAGS)

%.s.o: %.s
	$(AS) $< -o $@ -MD $<.d

%.c.libk.o: %.c
	$(CC) -c $< -o $@ -MD -MF $<.libk.d -MT $@ $(CCFLAGS) $(LIBK_CCFLAGS)

%.s.libk.o: %.s
	$(AS) $< -o $@ -MD $<.libk.d

-include $(OBJS:.o:.d) $(LIBK_OBJS:.o:.d)