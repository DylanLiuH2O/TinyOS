#ifndef __LIB_KERNEL_LIST_H
#define __LIB_KERNEL_LIST_H

#include "lib/stdint.h"

//通过成员的偏移量,求出结构体的起始地址
#define offset(struct_type,memeber) (uint32_t)(&((struct_type*)0)->member)
#define node2entry(struct_type,struct_memeber_name, node_ptr) \
        (struct_type *)((uint32_t)node_ptr - offset(struct_type, struct_member_name))

typedef uint8_t _BOOL;

struct list_node {
    struct list_node* prev;
    struct list_node* next;
};

struct list {
    struct list_node head;
    struct list_node tail;
};

typedef _BOOL (*function)(struct list_node*, uint32_t arg);

void list_init(struct list* alist);

void list_insert_before(struct list_node* before, struct list_node* node);
void list_remove(struct list_node* pnode);

void list_push(struct list* plist, struct list_node* node);
struct list_node* list_pop(struct list* plist);
void list_append(struct list* plist, struct list_node* node);

//void list_iterate(struct list* plist);

_BOOL list_empty(struct list* plist);
uint32_t list_len(struct list* plist);

struct list_node* list_traversal(struct list* plist, function func, uint32_t arg);
_BOOL node_find(struct list* plist, struct list_node* obj_node);

#endif 
