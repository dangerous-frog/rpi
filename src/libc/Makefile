# Inherit from parent
CC ?= aarch64-none-elf-gcc
CFLAGS += -Iinclude 
BUILDDIR ?= build

# Source files
STDIO_CFILES = $(wildcard stdio/*.c)

# Object files
STDIO_OBJS = $(addprefix $(BUILDDIR)/, $(notdir $(STDIO_CFILES:.c=.o)))

all: $(STDIO_OBJS)

# stdio files
$(BUILDDIR)/%.o: stdio/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	# Objects cleaned by parent

.PHONY: all clean