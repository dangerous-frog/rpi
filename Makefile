# Cross-compiler setup
GCCPATH = $(PWD)/arm-gnu-toolchain/bin
CC = $(GCCPATH)/aarch64-none-elf-gcc
LD = $(GCCPATH)/aarch64-none-elf-ld
OBJCOPY = $(GCCPATH)/aarch64-none-elf-objcopy

# 
export CC
export CFLAGS

# Build directories
BUILDDIR = build
SRCDIR = src

# Global flags
CFLAGS = -Wall -O2 -g -ffreestanding -nostdinc -nostdlib -nostartfiles
CFLAGS += -Isrc/kernel/include -Isrc/libc/include

all: kernel8.img

# Create build directory
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# Build kernel and libc
kernel8.img: $(BUILDDIR) 
	$(MAKE) -C $(SRCDIR)/kernel BUILDDIR=../../$(BUILDDIR)
	$(MAKE) -C $(SRCDIR)/libc BUILDDIR=../../$(BUILDDIR)
	$(LD) -nostdlib -T $(SRCDIR)/kernel/arch/aarch64/link.ld \
		$(BUILDDIR)/*.o -o $(BUILDDIR)/kernel8.elf
	$(OBJCOPY) -O binary $(BUILDDIR)/kernel8.elf kernel8.img

clean:
	rm -rf $(BUILDDIR) kernel8.img || true
	$(MAKE) -C $(SRCDIR)/kernel clean
	$(MAKE) -C $(SRCDIR)/libc clean

.PHONY: all clean