#ifndef DEVICE_IOQUEUE_H
#define DEVICE_IOQUEUE_H

#include "lib/stdint.h"
#include "thread/sync.h"

#define BUFSIZE 64

struct ioqueue {
    struct lock ioqueue_lock;
    struct semaphore products;
    struct semaphore freespace;
    char buffer[BUFSIZE];
    int32_t head;
    int32_t tail;
};

void ioqueue_init(struct ioqueue* aioqueue);
void ioqueue_putchar(struct ioqueue* aioqueue, char ch);
char ioqueue_getchar(struct ioqueue* aioqueue);

static int32_t next_pos(int32_t pos);
/*
static _BOOL ioqueue_full(struct ioqueue* aioqueue);
static _BOOL ioqueue_empty(struct ioqueue* aioqueue);
static void ioqueue_wait(struct task_struct** waiter);
static void ioqueue_wake(struct task_struct** waiter);
*/


#endif