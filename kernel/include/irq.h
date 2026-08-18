// SPDX-FileCopyrightText: 2026 Zhan Zixuan
//
// SPDX-License-Identifier: MIT

#ifndef __IRQ_H
#define __IRQ_H

#include "frame.h"

// irq 处理函数
typedef void (*irq_handler)(struct frame *);

void init_irq();
void register_irq(int irq, irq_handler handler);

// 向 PIC 发送 End Of Interrupt
void send_eoi();

#endif
