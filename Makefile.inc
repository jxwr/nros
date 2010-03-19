
CC = gcc
CFLAGS = -m32 \
        -fno-builtin \
	-fno-stack-protector \
	-nostdinc \
	-ggdb \
	-Wall
ASFLAGS = $(CFLAGS)

OBJCOPY = objcopy
OBJCOPY_FLAGS = -O binary -R .note -R .comment -S

LD = ld
LDFLAGS = -melf_i386 -static -nostdlib 