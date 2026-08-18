// SPDX-FileCopyrightText: 2026 Zhan Zixuan
//
// SPDX-License-Identifier: MIT

#ifndef __FRAME_H
#define __FRAME_H

struct frame {
    // 通用寄存器
    unsigned long di, si, bp, sp, bx, dx, cx, ax;
    // 段寄存器
    unsigned short gs, __gsh;
    unsigned short fs, __fsh;
    unsigned short es, __esh;
    unsigned short ds, __dsh;
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
} __attribute__((__packed__));

#endif
