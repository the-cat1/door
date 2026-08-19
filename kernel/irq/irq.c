// SPDX-FileCopyrightText: 2026 Zhan Zixuan
//
// SPDX-License-Identifier: MIT

// 处理 IDT 和 IRQ

#include <stddef.h>
#include <stdint.h>

#include "assert.h"
#include "boot.h"
#include "frame.h"
#include "printk.h"
#include "spin_lock.h"
#include "irq.h"

#define DESC_COUNT 256
#define DESC_ATTR_DPL0 0b10001110
#define DESC_ATTR_DPL3 0b11101110

// 中断门描述符
struct gate_desc {
    uint16_t offset_low;
    uint16_t seg_selector;
    uint8_t reserved;
    uint8_t attribute;
    uint16_t offset_high;
} __attribute__((packed));

extern const uint32_t irq_proc_table[DESC_COUNT]; // idt_proc.asm
extern void register_exceptions(); // exceptions.c
extern void init_pic();
extern void enable_pic_irq(int irq);

static struct gate_desc idt[DESC_COUNT];
static struct spin_lock irq_handlers_lock;
static irq_handler irq_handlers[DESC_COUNT];

/**
 * @brief irq 处理，寻找对应的回调函数并调用
 *
 * @param frame 栈帧
 * @note 此函数由 irq_proc.asm 调用
 */
void int_common(struct frame *frame)
{
    int irq = frame->irq;
    assert(0 <= irq && irq < DESC_COUNT);
    if (irq_handlers[irq])
        irq_handlers[irq](frame);
    else
        printk("no interrupt handler for %d", irq);
}

/**
 * @brief 注册一个 IRQ 回调函数
 *
 * @param irq 要注册的 IRQ 号
 * @param handler 处理函数
 */
void register_irq(int irq, irq_handler handler)
{
    assert(0 <= irq && irq < DESC_COUNT);
    assert(!irq_handlers[irq]);

    spin_lock_relase(&irq_handlers_lock);

    irq_handlers[irq] = handler;
    if (0x20 <= irq && irq < 0x30)
        enable_pic_irq(irq);

    spin_lock_relase(&irq_handlers_lock);
}

/**
 * @brief 初始化并加载 IDT，设置 pic
 *
 * @note 在关闭中断的情况下调用
 */
void init_irq()
{
    // 设置 IDT
    for (int i = 0; i < DESC_COUNT; i++) {
        uint32_t irq_proc = irq_proc_table[i];
        idt[i] = (struct gate_desc){
            .offset_low = irq_proc & 0x0000FFFF,
            .offset_high = irq_proc >> 16,
            .seg_selector = GDT_CODE_SEG,
            .attribute = DESC_ATTR_DPL0,
            .reserved = 0
        };
    }

    // 加载 LDTR
    uint64_t idtr = (uint64_t)(uint32_t)idt << 16 | (sizeof(idt) - 1);
    asm("lidt %0" ::"m"(idtr));

    spin_lock_init(&irq_handlers_lock);

    register_exceptions(); // 注册异常
    init_pic(); // 初始化 PIC

    printk("initialized irq");
}
