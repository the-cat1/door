// SPDX-FileCopyrightText: 2026 Zhan Zixuan
//
// SPDX-License-Identifier: MIT

// Some inline ASM operates.

#ifndef __ASM_H
#define __ASM_H

#include <stdint.h>

#define EFLAGS_IF 0x00000200

static inline void asm_hlt()
{
    asm("hlt");
}

static inline void asm_cli()
{
    asm("cli");
}

static inline void asm_sti()
{
    asm("sti");
}

static inline void asm_nop()
{
    asm("nop");
}

static inline void asm_pause()
{
    asm("pause");
}

static inline void asm_outb(uint16_t port, uint8_t data)
{
    asm("outb %%al, %%dx" ::"a"(data), "d"(port));
    asm_nop();
    asm_nop();
}

static inline uint8_t asm_inb(uint16_t port)
{
    uint8_t ret;
    asm("inb %%dx, %%al" : "=a"(ret) : "d"(port));
    asm_nop();
    asm_nop();
    return ret;
}

static inline unsigned int asm_ead_eflags()
{
    unsigned int flags;
    asm("pushf\n\t"
        "popl %0"
        : "=r"(flags));
    return flags;
}

static inline void asm_write_eflags(unsigned int flags)
{
    asm("pushl %0\n\t"
        "popf" ::"r"(flags));
}

static inline void asm_set_if(unsigned int val)
{
    val ? asm_sti() : asm_cli();
}

#endif
