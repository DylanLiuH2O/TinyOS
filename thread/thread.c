#include "thread.h"
#include "stdint.h"
#include "string.h"
#include "memory.h"

#define PG_SIZE 4096

//kernel_thread执行function(func_arg)
static void kernel_thread(thread_func* function, void* func_arg)
{
    function(func_arg);
}

//初始化线程基本信息
void init_thread(struct task_struct* pthread, char* name, int pri)
{
    memset(pthread, 0, sizeof(*pthread));  
    strcpy(pthread->name, name);
    pthread->status   = TASK_RUNNING;
    pthread->priority = pri;
    //self_kstack是线程在内核态下使用的栈顶地址
    pthread->self_kstack = (uint32_t*)((uint32_t)pthread + PG_SIZE);
    pthread->stack_magic = 0x19991005;
}

//初始化线程栈
void thread_create(struct task_struct* pthread, thread_func function, void* func_arg) 
{
    //预留中断栈空间
    pthread->self_kstack -= sizeof(struct intr_stack);

    //预留线程栈空间
    pthread->self_kstack -= sizeof(struct thread_stack);

    struct thread_stack* kthread_stack = (struct thread_stack*)pthread->self_kstack;
    kthread_stack->eip      = kernel_thread;
    kthread_stack->function = function;
    kthread_stack->func_arg = func_arg;
    kthread_stack->ebp      = kthread_stack->ebx = 0;
    kthread_stack->esi      = kthread_stack->edi = 0;
}

//执行线程
struct task_struct* thread_start(char* name, int pri, thread_func function, void* func_arg)
{
    struct task_struct* thread = get_kernel_pages(1);

    init_thread(thread, name, pri);
    thread_create(thread, function, func_arg);
    asm volatile ("movl %0, %%esp; pop %%ebp; \
                   pop %%ebx; pop %%edi; pop %%esi; ret" : : "g" (thread->self_kstack) : "memory");

    return thread;
}
