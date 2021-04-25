#include "kernel/interrupt.h"
#include "lib/kernel/list.h"

#define NULL 0

void list_init(struct list* alist)
{
    alist->head.prev = NULL;
    alist->head.next = &alist->tail;
    alist->tail.next = NULL;
    alist->tail.prev = &alist->head;
}

void list_insert_before(struct list_node* before, struct list_node* node)
{
    //关中断,保证操作的原子性
    enum intr_status old_status = intr_disable();

    before->prev->next = node;
    node->prev         = before->prev;
    node->next         = before;
    before->prev       = node;

    //恢复关中断前的状态
    set_intr_status(old_status);
}

void list_remove(struct list_node* pnode)
{
    //关中断,保证操作的原子性
    enum intr_status old_status = intr_disable();

    pnode->prev->next = pnode->next;
    pnode->next->prev = pnode->prev;

    //恢复关中断前的状态
    set_intr_status(old_status);
}

void list_push(struct list* plist, struct list_node* node)
{
    list_insert_before(plist->head.next, node);
}

struct list_node* list_pop(struct list* plist)
{
    struct list_node* node = plist->head.next;
    list_remove(node);    
    return node;
}

/*
void list_iterate(struct list* plist)
{
    
}
*/

void list_append(struct list* plist, struct list_node* node)
{
    list_insert_before(&plist->tail, node);
}

_BOOL list_empty(struct list* plist)
{
    return plist->head.next == &plist->tail ? 1 : 0;
}

uint32_t list_len(struct list* plist)
{
    struct list_node* curr = plist->head.next;
    uint32_t len = 0;

    while (curr != &plist->tail) {
        len++;
        curr = curr->next;
    }
    return len;
}

//遍历链表,arg和node作为参数执行回调函数func,判断node是否符合要求
struct list_node* list_traversal(struct list* plist, function func, uint32_t arg)
{
    if (list_empty(plist) == 0)  {
        return NULL;
    }

    struct list_node* curr = plist->head.next;
    while (curr != &plist->tail) {
        if (func(curr, arg) == 1) {
            return curr; 
        }
        curr = curr->next;
    }
    return NULL;
}

_BOOL node_find(struct list* plist, struct list_node* obj_node)
{
    struct list_node* curr = plist->head.next;
    while (curr != &plist->tail) {
        if (curr == obj_node)  {
            return 1; 
        }
        curr = curr->next;
    }
    
    return 0;
}
