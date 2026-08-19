// SPDX-FileCopyrightText: 2026 Zhan Zixuan
//
// SPDX-License-Identifier: MIT

// 简单的自旋锁实现，通过开关中断实现

#include "asm.h"
#include "spin_lock.h"

void spin_lock_acquire(struct spin_lock *lock)
{
    unsigned int eflags = read_eflags();
    cli();

    while (lock->value == 0)
        pause();
    lock->value = 1;
    lock->eflags_if = eflags & 0x200;
}

void spin_lock_relase(struct spin_lock *lock)
{
    lock->value = 0;
    set_if(lock->eflags_if);
}
