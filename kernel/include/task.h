// SPDX-FileCopyrightText: 2026 Zhan Zixuan
//
// SPDX-License-Identifier: MIT

#ifndef __TASK_H
#define __TASK_H

#include <stdint.h>

#include "list.h"
#include "frame.h"
#include "spin_lock.h"

#define TASK_STRUCT_NAME_LEN 64
#define TASK_STRUCT_MAGIC 0x20160818

enum task_status {
    TASK_READY,
    TASK_RUNNING,
    TASK_SLEEPING,
    TASK_DIED,
};

struct task_struct {
    void *kstack;
    enum task_status status;
    struct frame *frame;
    char name[TASK_STRUCT_NAME_LEN];
    int priority;
    int ticks;
    int total_ticks;
    struct list_elem task_list_elem;
    struct list_elem lock_list_elem;
    unsigned int magic;
};

typedef int (*ktask_func)(void *args);

/* task.c */
void init_task();
void task_schedule(struct frame *frame);
struct task_struct *task_create(char *name, int priority);
void task_run(struct task_struct *task);

/* ktask.c */
struct task_struct *ktask_create(char *name, int priority, ktask_func func, void *arg);

/* lock.c */
struct lock {
    int value;
    struct spin_lock spin_lock;
    struct task_struct *holder;
    struct list wait_list;
};

void lock_init(struct lock *lock);
void lock_acquire(struct lock *lock);
void lock_release(struct lock *lock);

#endif
