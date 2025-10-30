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
CFLAGS = -Wall -O0 -g3 -ffreestanding -nostdlib -nostartfiles
CFLAGS += -Isrc/kernel/include -Isrc/libc/include

# Dependency tracking
KERNEL_SRCS = $(shell find $(SRCDIR)/kernel -name '*.c' -o -name '*.S')
LIBC_SRCS = $(shell find $(SRCDIR)/libc -name '*.c' -o -name '*.S')
ALL_SRCS = $(KERNEL_SRCS) $(LIBC_SRCS)


all: kernel8.img build/user_debug.elf

# Create build directory
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# Build kernel and libc
kernel8.img: $(BUILDDIR) $(ALL_SRCS) $(SRCDIR)/kernel/arch/aarch64/link.ld
	$(MAKE) -C $(SRCDIR)/kernel BUILDDIR=../../$(BUILDDIR)
	$(MAKE) -C $(SRCDIR)/libc BUILDDIR=../../$(BUILDDIR)
	$(LD) -nostdlib --allow-multiple-definition --warn-common \
      -T $(SRCDIR)/kernel/arch/aarch64/link.ld \
      $(BUILDDIR)/*.o $(BUILDDIR)/*/*.o -o $(BUILDDIR)/kernel8.elf
	$(OBJCOPY) -O binary $(BUILDDIR)/kernel8.elf kernel8.img

build/user_debug.elf: build/kernel8.elf
	$(OBJCOPY) --only-section=.text.user \
	           --only-section=.rodata.user \
	           --only-section=.bss.user \
	           --only-section=.data.user \
	           --only-section=.debug_* \
	           --change-section-lma .text.user=0x0 \
	           --change-section-vma .text.user=0x0 \
	           --change-section-lma .rodata.user=0x900 \
	           --change-section-vma .rodata.user=0x900 \
	           build/kernel8.elf build/user_debug.elf

clean:
	rm -rf $(BUILDDIR) kernel8.img || true
	$(MAKE) -C $(SRCDIR)/kernel clean
	$(MAKE) -C $(SRCDIR)/libc clean

.PHONY: all clean