// SPDX-FileCopyrightText: 2026 Zhan Zixuan
//
// SPDX-License-Identifier: MIT

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "asm.h"
#include "lib/string.h"
#include "boot.h"
#include "frame.h"
#include "list.h"
#include "mm.h"
#include "panic.h"
#include "printk.h"
#include "task.h"

void task_switch_to(struct task_struct *);

struct list task_list;
struct task_struct *cur_task;
struct task_struct *idle_task;

static void idle_task_func()
{
    while (true)
        hlt();
}

struct task_struct *task_create(char *name, int priority, uintptr_t start_ip)
{
    struct task_struct *task = mm_page_alloc_k(1);
    if (!task)
        return NULL;

    memset(task, 0, 4096);
    strncpy(task->name, name, TASK_STRUCT_NAME_LEN - 1);
    task->priority = priority;
    task->magic = TASK_STRUCT_MAGIC;
    task->kstack = (void *)((uintptr_t)task + 4096);
    task->frame = (void *)((uintptr_t)task + 4096 - sizeof(struct frame));
    task->frame->cs = GDT_CODE_SEG;
    task->frame->ds = GDT_DATA_SEG;
    task->frame->es = GDT_DATA_SEG;
    task->frame->fs = GDT_DATA_SEG;
    task->frame->gs = GDT_DATA_SEG;
    task->frame->ip = start_ip;
    task->frame->flags = read_eflags() | 0x00000200; // open if
    task->status = TASK_READY;
    list_push(&task_list, &task->elem);

    return task;
}

void task_run(struct task_struct *task)
{
    task->status = TASK_RUNNING;
}

void task_schedule(struct frame *frame)
{
    if (list_empty(&task_list))
        return;

    struct list_elem *next_task_elem = task_list.head.next;
    if (cur_task) {
        cur_task->total_ticks++;
        cur_task->ticks--;
        if (cur_task->ticks > 0)
            return; // continue run current task

        // need task switch
        if (cur_task->elem.next->next != NULL)
            next_task_elem = cur_task->elem.next;
    }

    // 换任务
    struct task_struct *next_task = list_entry(struct task_struct, elem, next_task_elem);
    while (!(next_task->status == TASK_RUNNING)) {
        next_task_elem = next_task_elem->next;
        if (next_task_elem->next == NULL)
            panic("no task to run, is there an idle task?");

        next_task = list_entry(struct task_struct, elem, next_task_elem);
    }

    cur_task->frame = frame;
    next_task->ticks = next_task->priority;
    cur_task = next_task;
    task_switch_to(next_task);
}

void init_task()
{
    list_init(&task_list);
    idle_task = task_create("idle", 1, (uintptr_t)idle_task_func);
    assert(idle_task);
    task_run(idle_task);
    printk("initialized task");
}
