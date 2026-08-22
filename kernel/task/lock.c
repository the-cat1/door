// SPDX-FileCopyrightText: 2026 Zhan Zixuan
//
// SPDX-License-Identifier: MIT

#include "list.h"
#include "spin_lock.h"
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
    spin_lock_init(&lock->spin_lock);
}

void lock_acquire(struct lock *lock)
{
    spin_lock_acquire(&lock->spin_lock);
    while (lock->value != 0) {
        list_append(&lock->wait_list, &cur_task->lock_list_elem);
        cur_task->status = TASK_SLEEPING;
        spin_lock_relase(&lock->spin_lock);
        task_schedule_now(); // 被唤醒的话，就是有任务释放了锁
        spin_lock_acquire(&lock->spin_lock);
    }

    lock->value = 1;
    lock->holder = cur_task;
    spin_lock_relase(&lock->spin_lock);
}

void lock_release(struct lock *lock)
{
    spin_lock_acquire(&lock->spin_lock);
    assert(lock->holder == cur_task);
    lock->value = 0;
    task_unblock_one(lock);
    spin_lock_relase(&lock->spin_lock);
}
