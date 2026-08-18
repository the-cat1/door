// SPDX-FileCopyrightText: 2026 Zhan Zixuan
//
// SPDX-License-Identifier: MIT

#ifndef __TASK_H
#define __TASK_H

#include <stdint.h>

#include "list.h"
#include "frame.h"

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
    struct list_elem elem;
    unsigned int magic;
};

void init_task();
void task_schedule(struct frame *frame);
struct task_struct *task_create(char *name, int priority, uintptr_t start_ip);
void task_run(struct task_struct *task);

#endif
