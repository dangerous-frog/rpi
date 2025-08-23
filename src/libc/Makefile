# Inherit from parent
CC ?= aarch64-none-elf-gcc
CFLAGS += -Iinclude 
BUILDDIR ?= build

# Source files
STDIO_CFILES = $(wildcard stdio/*.c)
STRING_CFILES = $(wildcard string/*.c)

# Object files
STDIO_OBJS = $(addprefix $(BUILDDIR)/, $(notdir $(STDIO_CFILES:.c=.o)))
STRING_OBJS = $(addprefix $(BUILDDIR)/, $(notdir $(STRING_CFILES:.c=.o)))

all: $(STDIO_OBJS) $(STRING_OBJS)

# stdio files
$(BUILDDIR)/%.o: stdio/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# string files  
$(BUILDDIR)/%.o: string/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	# Objects cleaned by parent

.PHONY: all clean