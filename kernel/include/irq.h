// kernel/include/interrupt.h

#ifndef __INTERRUPT_H
#define __INTERRUPT_H

struct irq_frame {
    // 通用寄存器
    unsigned long ax, bx, cx, dx, si, di, sp, bp;
    // 段寄存器
    unsigned short ss, __ssh;
    unsigned short ds, __dsh;
    unsigned short es, __esh;
    unsigned short fs, __fsh;
    unsigned short gs, __gsh;
    // 由 irq_proc 压入的内容
    int irq;
    unsigned long errorcode;
    // cpu 压入的内容
    unsigned long ip;
    unsigned short cs, __csh;
    unsigned long flags;
    // 以下的内容只有在有特权级转换时才出现
    unsigned long org_sp;
    unsigned short org_ss, __org_ssh;
};

// irq 回调
typedef void (*irq_callback)(struct irq_frame *);

void init_irq();
void register_irq(int irq, irq_callback cb);

#endif
