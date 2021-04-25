#ifndef __THREAD_SWITCH_H
#define __THREAD_SWITCH_H

#include "thread/thread.h"

void switch_to(struct task_struct* curr_thread, struct task_struct* next_thread);

#endif
