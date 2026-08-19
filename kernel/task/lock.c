// SPDX-FileCopyrightText: 2026 Zhan Zixuan
//
// SPDX-License-Identifier: MIT

#include "asm.h"
#include "list.h"
#include "task.h"
#include <assert.h>
#include <stdbool.h>

extern struct task_struct *cur_task;

static void task_unblock_one(struct lock *lock)
{
    struct list_elem *elem = list_pop(&lock->wait_list);
    if (elem) {
        struct task_struct *task = list_entry(struct task_struct, lock_list_elem, elem);
        task->status = TASK_RUNNING;
    }
}

void lock_init(struct lock *lock)
{
    lock->holder = NULL;
    lock->value = 0;
    list_init(&lock->wait_list);
}

void lock_acquire(struct lock *lock)
{
    unsigned long eflags = read_eflags();
    cli();

    while (lock->value != 0) {
        list_append(&lock->wait_list, &cur_task->lock_list_elem);
        cur_task->status = TASK_SLEEPING;
        write_eflags(eflags);
        hlt(); // 被唤醒的话，就是有任务释放了锁
        cli();
    }

    lock->value = 1;
    lock->holder = cur_task;
    write_eflags(eflags);
    return;
}

void lock_release(struct lock *lock)
{
    unsigned long eflags = read_eflags();
    cli();
    assert(lock->holder == cur_task);
    lock->value = 0;
    task_unblock_one(lock);
    write_eflags(eflags);
}
