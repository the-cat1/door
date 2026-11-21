# SPDX-FileCopyrightText: 2026 Zhan Zixuan
#
# SPDX-License-Identifier: MIT


include ../Makefile.config


BOOTSECT_BIN    = bootsect.bin
LOADER_C_SRCS   = loader.c
LOADER_ASM_SRCS = entry.asm
LOADER_OBJS     = $(patsubst %.c, %.o, $(LOADER_C_SRCS)) \
                  $(patsubst %.asm, %.o, $(LOADER_ASM_SRCS))


all : $(BOOTSECT_BIN) $(LOADER_OBJS)
	$(CC) $(LDFLAGS) -T linker.ld -o loader.elf $(LOADER_OBJS)
	$(OBJCOPY) -O binary loader.elf loader.bin

clean:
	$(RM) *.bin *.lst *.o *.elf

.PHONY : all clean
