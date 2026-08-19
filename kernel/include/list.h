// SPDX-FileCopyrightText: 2026 Zhan Zixuan
//
// SPDX-License-Identifier: MIT

#ifndef __LIST_H
#define __LIST_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "spin_lock.h"

#define list_entry(structname, elem_name, elem) ((structname *)((uintptr_t)(elem) - offsetof(structname, elem_name)))

struct list_elem {
    struct list_elem *prev;
    struct list_elem *next;
};

struct list {
    struct list_elem head;
    struct list_elem tail;
    struct spin_lock lock;
};

typedef bool(list_traversal_func)(struct list_elem *);

void list_init(struct list *l);
void list_push(struct list *l, struct list_elem *elem);
struct list_elem *list_pop(struct list *l);
void list_append(struct list *l, struct list_elem *elem);
struct list_elem *list_remove(struct list *l, struct list_elem *elem);
size_t list_len(struct list *l);
bool list_empty(struct list *l);
struct list_elem *list_traversal(struct list *l, list_traversal_func f);

#endif /* __LIST_H */
