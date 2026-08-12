// kernel/irq/exceptions.c
// 处理 CPU 内部的中断（异常）

#include <stddef.h>

#include "array_size.h"
#include "panic.h"
#include "irq.h"

struct exception_info {
    char *mnemonic;
    char *description;
};

#define RESERVED (struct exception_info){NULL, "Intel Reserved"}

static const struct exception_info exceptions[] = {
    {"#DE", "Divide Error"},
    {"#DB", "Debug Exception"},
    {NULL, "NMI Interrupt"},
    {"#BP", "Breakpoint"},
    {"#OF", "Overflow"},
    {"#BR", "BOUND Range Exceeded"},
    {"#UD", "Invalid Opcode"},
    {"#NM", "Device Not Available"},
    {"#DF", "Double Fault"},
    RESERVED,
    {"#TS", "Invalid TSS"},
    {"#NP", "Segment Not Present"},
    {"#SS", "Stack Fault"},
    {"#GP", "General Protection"},
    {"#PF", "Page Fault"},
    RESERVED,
    {"#MF", "Floating-Point Error"},
    {"#AC", "Alignment Check"},
    {"#MC", "Machine Check"}
};

static void general_irq(struct irq_frame *frame)
{
    struct exception_info info = RESERVED;

    if (frame->irq >= 0 && frame->irq < (int)array_size(exceptions))
        info = exceptions[frame->irq];

    panic("exception: (IRQ%d) %s", frame->irq, info.description);
}

/**
 * @brief 注册异常的处理函数
 *
 * @note 在 init_irq(irq.c) 中调用
 */
void register_exceptions()
{
    for (int i = 0; i < 32; i++)
        register_irq(i, general_irq);
}
