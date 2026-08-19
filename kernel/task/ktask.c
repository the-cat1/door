// SPDX-FileCopyrightText: 2026 Zhan Zixuan
//
// SPDX-License-Identifier: MIT

#include <stdint.h>

#include "asm.h"
#include "boot.h"
#include "task.h"

static void ktask_start()
{
    unsigned long func;
    void *arg;
    asm("movl %%edi, %0\n\t"
        "movl %%esi, %1"
        : "=r"(func), "=r"(arg));
    ((ktask_func)func)(arg);
}

struct task_struct *ktask_create(char *name, int priority, ktask_func func, void *arg)
{
    struct task_struct *task = task_create(name, priority);
    if (!task)
        return NULL;

    task->frame->di = (unsigned long)func;
    task->frame->si = (unsigned long)arg;
    task->frame->cs = GDT_CODE_SEG;
    task->frame->ds = GDT_DATA_SEG;
    task->frame->es = GDT_DATA_SEG;
    task->frame->fs = GDT_DATA_SEG;
    task->frame->gs = GDT_DATA_SEG;
    task->frame->flags = EFLAGS_IF;
    task->frame->ip = (unsigned long)ktask_start;

    return task;
}
