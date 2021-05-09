#include "device/ioqueue.h"
#include "kernel/global.h"
#include "device/console.h"

void ioqueue_init(struct ioqueue* aioqueue)
{
    lock_init(&aioqueue->ioqueue_lock);
    sema_init(&aioqueue->products, 0);
    sema_init(&aioqueue->freespace, BUFSIZE);
    aioqueue->head = 0;
    aioqueue->tail = 0;
}

//producer
void ioqueue_putchar(struct ioqueue* aioqueue, char ch)
{
    sema_down(&aioqueue->freespace);
    lock_acquire(&aioqueue->ioqueue_lock);

    /*
    console_put_str("== producer ==\n");
    console_put_str("free: ");
    console_put_int(aioqueue->freespace.value);
    console_put_str("\n");
    console_put_str("products: ");
    console_put_int(aioqueue->products.value);
    console_put_str("\n\n");
    */

    int32_t pos = aioqueue->head;
    aioqueue->buffer[pos] = ch;
    aioqueue->head = next_pos(pos);

    lock_release(&aioqueue->ioqueue_lock);
    sema_up(&aioqueue->products);
}

//consumer
char ioqueue_getchar(struct ioqueue* aioqueue)
{
    sema_down(&aioqueue->products);
    lock_acquire(&aioqueue->ioqueue_lock);

    /*
    console_put_str("== consumer ==\n");
    console_put_str("free: ");
    console_put_int(aioqueue->freespace.value);
    console_put_str("\n");
    console_put_str("products: ");
    console_put_int(aioqueue->products.value);
    console_put_str("\n\n");
    */

    int32_t pos = aioqueue->tail;
    char ret_val = aioqueue->buffer[pos];
    aioqueue->tail = next_pos(pos);

    lock_release(&aioqueue->ioqueue_lock);
    sema_up(&aioqueue->freespace);

    return ret_val;
}

int32_t next_pos(int32_t pos)
{
    return (pos + 1) % BUFSIZE;
}

/*
_BOOL ioqueue_full(struct ioqueue* aioqueue)
{
    return next_pos(aioqueue->head) == aioqueue->tail;
}

_BOOL ioqueue_empty(struct ioqueue* aioqueue)
{
    return aioqueue->head == aioqueue->tail;
}

void ioqueue_wait(struct task_struct** waiter)
{
    *waiter = running_thread();
    thread_block(TASK_BLOCKED);
}

void ioqueue_wake(struct task_struct** waiter)
{
    thread_unblock(*waiter);
}
*/
