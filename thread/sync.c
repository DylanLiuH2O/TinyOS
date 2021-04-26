#include "thread/sync.h"
#include "lib/kernel/list.h"
#include "kernel/interrupt.h"
#include "kernel/debug.h"

#define NULL 0

void sema_init(struct semaphore* sema, uint8_t value)
{
    sema->value = value;
    list_init(&sema->wait_list);
}

void lock_init(struct lock* alock)
{
    alock->holder = NULL;
    sema_init(&alock->semaphore, 1);
    alock->holder_repeat_num = 0;
}

void sema_down(struct semaphore* sema)
{
    enum intr_status old_status = intr_disable();
    struct task_struct* curr_thread = running_thread();

    while (sema->value == 0) {
        ASSERT(node_find(&sema->wait_list, &curr_thread->general_tag) == 0);
        list_append(&sema->wait_list, &curr_thread->general_tag);
        thread_block(TASK_BLOCKED);
    }
    sema->value--;
    set_intr_status(old_status);
}

void sema_up(struct semaphore* sema)
{
    enum intr_status old_status = intr_disable();
    ASSERT(sema->value == 0);
    if (list_empty(&sema->wait_list) == 0) {
        struct list_node* blocked_thread_tag = list_pop(&sema->wait_list);
        struct task_struct* blocked_thread = (struct task_struct*) \
                                        ((uint32_t)blocked_thread_tag - \
                                        (uint32_t)(&((struct task_struct*)0x0)->general_tag));
        thread_unblock(blocked_thread);
    }
    sema->value++;
    set_intr_status(old_status);
}

void lock_acquire(struct lock* alock)
{
    struct task_struct* curr_thread = running_thread();
    if (alock->holder != curr_thread) {
        sema_down(&alock->semaphore);
        alock->holder = curr_thread;
        alock->holder_repeat_num = 0;
    } else {
        alock->holder_repeat_num++;
    }
}

void lock_release(struct lock* alock)
{
    struct task_struct* curr_thread = running_thread();
    ASSERT(alock->holder == curr_thread);
    if (alock->holder_repeat_num > 1) {
        alock->holder_repeat_num--;
        return;
    }
    alock->holder = NULL;
    alock->holder_repeat_num = 0;
    sema_up(&alock->semaphore);
}