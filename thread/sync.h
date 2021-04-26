#ifndef __THREAD_SYNC_H
#define __THREAD_SYNC_H

#include "lib/stdint.h"
#include "lib/kernel/list.h"
#include "thread/thread.h"

struct semaphore {
    uint8_t value;
    struct list wait_list;
};

struct lock {
    struct task_struct* holder;
    struct semaphore semaphore;
    uint32_t holder_repeat_num;
};

void sema_init(struct semaphore* sema, uint8_t value);
void lock_init(struct lock* alock);
void sema_down(struct semaphore* sema);
void sema_up(struct semaphore* sema);
void lock_acquire(struct lock* alock);
void lock_release(struct lock* alock);

#endif
