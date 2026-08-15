# SPDX-FileCopyrightText: 2026 Zhan Zixuan
#
# SPDX-License-Identifier: MIT

include ../Makefile.config

TARGET		= kernel
LDSRCIPT	= linker.ld
CSOURCES	= $(shell find . -iname "*.c")
ASSOURCES	= $(shell find . -iname "*.asm")
OBJS		= $(patsubst %.c, %.o, $(CSOURCES)) \
			  $(patsubst %.asm, %.o, $(ASSOURCES))

CFLAGS		+= -I include

all : $(TARGET)

$(TARGET) : $(OBJS) $(LDSRCIPT)
	$(CC) -T $(LDSRCIPT) $(OBJS) -o $@ $(LDFLAGS)

clean:
	$(RM) $(OBJS) $(TARGET)
