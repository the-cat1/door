// SPDX-FileCopyrightText: 2026 Zhan Zixuan
//
// SPDX-License-Identifier: MIT

#ifndef __SPIN_LOCK_H
#define __SPIN_LOCK_H

struct spin_lock {
    int value;
    unsigned int eflags_if;
};

void spin_lock_acquire(struct spin_lock *lock);
void spin_lock_relase(struct spin_lock *lock);

#endif
