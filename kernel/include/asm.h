// kernel/include/asm.h
// 包括一些汇编操作

#ifndef __ASM_H
#define __ASM_H

#include <stdint.h>

static inline void hlt()
{
    asm("hlt");
}

static inline void cli()
{
    asm("cli");
}

static inline void sti()
{
    asm("sti");
}

static inline void nop()
{
    asm("nop");
}

static inline void outb(uint16_t port, uint8_t data)
{
    asm("outb %%al, %%dx" ::"a"(data), "d"(port));
    nop();
    nop();
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm("inb %%dx, %%al" : "=a"(ret) : "d"(port));
    nop();
    nop();
    return ret;
}

#endif
