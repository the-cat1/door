// SPDX-FileCopyrightText: 2026 Zhan Zixuan
//
// SPDX-License-Identifier: MIT

#include <stdint.h>

#include "asm.h"
#include "gdt.h"
#include "task.h"

struct task_struct *utask_create(const char *name, int priority, uintptr_t ip, uintptr_t sp)
{
    struct task_struct *task = task_create(name, priority);

    task->frame->ip = ip;
    task->frame->cs = selector_user_code;
    task->frame->ds = selector_user_data;
    task->frame->es = selector_user_data;
    task->frame->fs = selector_user_data;
    task->frame->gs = selector_user_data;
    task->frame->org_sp = sp;
    task->frame->org_ss = selector_user_data;
    task->frame->flags = EFLAGS_IF;

    return task;
}
