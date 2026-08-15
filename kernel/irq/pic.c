// SPDX-FileCopyrightText: 2026 Zhan Zixuan
//
// SPDX-License-Identifier: MIT

// 设置 PIC

#include <stdint.h>

#include "asm.h"
#include "assert.h"
#include "printk.h"
#include "irq.h"

#define PIC1_CTRL 0x20
#define PIC1_DATA 0x21
#define PIC2_CTRL 0xa0
#define PIC2_DATA 0xa1

static uint16_t pic_mask;

static void set_pic_mask()
{
    outb(PIC1_DATA, pic_mask & 0xff);
    outb(PIC2_DATA, pic_mask >> 8);
}

/**
 * @brief 发送 EOI 到 pic
 *
 * @note 需要在中断中调用
 */
void send_eoi()
{
    outb(PIC1_CTRL, 0x20);
    outb(PIC2_CTRL, 0x20);
}

void enable_pic_irq(int irq)
{
    assert(irq >= 0x20 && irq < 0x30);
    pic_mask &= ~(1 << (irq - 0x20)); // 启用对应的中断
    set_pic_mask();
}

/**
 * @brief 初始化 pic
 *
 * @note 在 init_irq 内调用
 */
void init_pic()
{
    pic_mask = 0xffff; // 先屏蔽所有中断
    set_pic_mask();

    // 初始化 PIC1
    outb(PIC1_CTRL, 0x11);
    outb(PIC1_DATA, 0x20);
    outb(PIC1_DATA, 0x04);
    outb(PIC1_DATA, 0x01);

    // 初始化 PIC2
    outb(PIC2_CTRL, 0x11);
    outb(PIC2_DATA, 0x28);
    outb(PIC2_DATA, 0x02);
    outb(PIC2_DATA, 0x01);

    pic_mask = 0xfffb; // 仅打开 pic2 的中断
    set_pic_mask();
}
