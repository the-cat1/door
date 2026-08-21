// SPDX-FileCopyrightText: 2026 Zhan Zixuan
//
// SPDX-License-Identifier: MIT

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "asm.h"
#include "gdt.h"
#include "lib/string.h"
#include "frame.h"
#include "list.h"
#include "mm.h"
#include "printk.h"
#include "task.h"

void task_switch_to(struct task_struct *);

struct list task_list;
struct task_struct *cur_task;
struct task_struct *idle_task;

/**
 * @berif idle 任务
 */
static void idle_task_func()
{
    while (true)
        hlt();
}

/**
 * @berif 创建一个任务
 *
 * @param name 任务名
 * @param priority 优先级，大于 0
 * @return 创建的 task
 *
 * @note 如果创建失败，返回 NULL
 * @warning 创建之后，需要设置 frame 作为任务开始运行的状态
 */
struct task_struct *task_create(char *name, int priority)
{
    struct task_struct *task = alloc_page_k(1);
    if (!task)
        return NULL;

    memset(task, 0, 4096);
    strncpy(task->name, name, TASK_STRUCT_NAME_LEN - 1);
    task->priority = priority;
    task->magic = TASK_STRUCT_MAGIC;
    task->kstack = (void *)((uintptr_t)task + 4096);
    task->frame = (void *)((uintptr_t)task + 4096 - sizeof(struct frame));
    task->status = TASK_READY;
    list_push(&task_list, &task->task_list_elem);

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
        if (cur_task->ticks > 0 && cur_task->status == TASK_RUNNING)
            return; // continue run current task

        // need task switch
        if (cur_task->task_list_elem.next->next != NULL) // 如果没到达链表尾部，设置 next_task 为下一个
            next_task_elem = cur_task->task_list_elem.next;
    }

    // 换任务
    struct task_struct *next_task = list_entry(struct task_struct, task_list_elem, next_task_elem);
    while (!(next_task->status == TASK_RUNNING)) {
        next_task_elem = next_task_elem->next;
        if (next_task_elem->next == NULL)
            next_task_elem = &idle_task->task_list_elem; // 到达链表尾部，设置为 idle

        next_task = list_entry(struct task_struct, task_list_elem, next_task_elem);
    }

    cur_task->frame = frame;
    next_task->ticks = next_task->priority;
    cur_task = next_task;
    task_switch_to(next_task);
}

void init_task()
{
    list_init(&task_list);
    idle_task = task_create("idle", 1);
    assert(idle_task);

    idle_task->frame->cs = selector_kernel_code;
    idle_task->frame->ip = (unsigned long)idle_task_func;
    idle_task->frame->flags = 0x00000200;
    task_run(idle_task);

    printk("initialized task");
}
