// SPDX-FileCopyrightText: 2026 Zhan Zixuan
//
// SPDX-License-Identifier: MIT

#include <stdbool.h>
#include <stddef.h>

#include "list.h"

/**
 * @berif 初始化一个链表
 *
 * @param l 要初始化的链表
 */
void list_init(struct list *l)
{
    l->head.prev = NULL;
    l->head.next = &l->tail;
    l->tail.prev = &l->head;
    l->tail.next = NULL;
}

/**
 * @berif 在链表的头部添加一个元素
 *
 * @param l 链表
 * @param elem 元素
 */
void list_push(struct list *l, struct list_elem *elem)
{
    elem->prev = &l->head; // 更新 elem 的 prev 和 next
    elem->next = l->head.next;
    l->head.next->prev = elem; // 更新原先第一项的 prev
    l->head.next = elem; // 更新 head 的 next
}

/**
 * @berif 在链表尾部添加一个元素
 *
 * @param l 链表
 * @param elem 要添加的元素
 */
void list_append(struct list *l, struct list_elem *elem)
{
    elem->prev = l->tail.prev;
    elem->next = &l->tail;
    l->tail.prev->next = elem;
    l->tail.prev = elem;
}

/**
 * @berif 在链表顶部弹出一个元素
 *
 * @param l 链表
 * @return 弹出的元素的指针
 */
struct list_elem *list_pop(struct list *l)
{
    struct list_elem *ret = l->head.next; // 保存 pop 出来的 list_elem
    if (ret->next == NULL) // 是否是 tail？
        return NULL;

    l->head.next = ret->next; // 设置 l->head
    ret->next->prev = &l->head; // 设置原来的第二项
    return ret;
}

/**
 * @berif 从链表中删除一个元素
 *
 * @param elem 要删除的元素
 * @return 被删除的元素，如果 elem 无效，返回 NULL
 */
struct list_elem *list_remove(struct list_elem *elem)
{
    if (elem == NULL || elem->next == NULL || elem->prev == NULL)
        return NULL;

    elem->prev->next = elem->next;
    elem->next->prev = elem->prev;

    return elem;
}

/**
 * @berif 检查链表是不是空的
 *
 * @param l 链表
 * @return 链表是否空的
 */
bool list_empty(struct list *l)
{
    return l->head.next == &l->tail && l->tail.prev == &l->head;
}

/**
 * @berif 计算链表的长度
 *
 * @param l 链表
 * @return 链表长度
 */
size_t list_len(struct list *l)
{
    size_t count = 0;
    struct list_elem *elem = l->head.next;
    while (elem->next != NULL) {
        elem = elem->next;
        count++;
    }

    return count;
}

/**
 * @berif 对链表中的每一个元素调用同函数，并返回该函数第一次返回 true 的元素
 *
 * @param l 链表
 * @param f 函数
 * @return `f` 第一次返回 `true` 的元素
 *
 * @note 如果未找到，返回 NULL
 */
struct list_elem *list_traversal(struct list *l, list_traversal_func f)
{
    struct list_elem *elem = l->head.next;
    while (elem->next != NULL) {
        if (f(elem))
            return elem;
        elem = elem->next;
    }

    return NULL; // no found
}
