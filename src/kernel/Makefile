# Inherit from parent
CFLAGS += -Iinclude -I../libc/include
BUILDDIR ?= build

# Source files
CFILES = $(wildcard *.c)
ARCH_CFILES = $(wildcard arch/aarch64/*.c)
ASMFILES = $(wildcard arch/aarch64/*.S)

# Object files
COBJS = $(addprefix $(BUILDDIR)/, $(CFILES:.c=.o))
ARCH_COBJS = $(addprefix $(BUILDDIR)/, $(notdir $(ARCH_CFILES:.c=.o)))
ASMOBJS = $(addprefix $(BUILDDIR)/, $(notdir $(ASMFILES:.S=.o)))

all: $(COBJS) $(ARCH_COBJS) $(ASMOBJS)

# Kernel C files
$(BUILDDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Architecture-specific C files  
$(BUILDDIR)/%.o: arch/aarch64/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Assembly files
$(BUILDDIR)/%.o: arch/aarch64/%.S
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	# Objects cleaned by parent

.PHONY: all clean