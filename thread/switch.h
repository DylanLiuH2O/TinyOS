#ifndef __SWITCH_H
#define __SWITCH_H

#include "thread.h"

void switch_to(struct task_struct* curr_thread, struct task_struct* next_thread);


#endif
