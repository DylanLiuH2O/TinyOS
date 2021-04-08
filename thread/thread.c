#include "thread.h"
#include "stdint.h"
#include "string.h"
#include "memory.h"
#include "interrupt.h"
#include "list.h"
#include "debug.h"
#include "switch.h"
#include "print.h"


#define NULL 0
#define PG_SIZE 4096

struct task_struct* main_thread;        //主线程
struct list thread_ready_list;          //就绪队列
struct list thread_all_list;            //所有队列
static struct list_node* thread_tag;    //线程tag

//任务切换函数,使用汇编实现
extern void switch_to(struct task_struct* curr, struct task_struct* next);

//获取当前运行线程的pcb指针
struct task_struct* running_thread()
{
   uint32_t esp;
   asm volatile ("mov %%esp, %0" : "=g" (esp));
   //抹去页内偏移量
   return (struct task_struct*)(esp & 0xfffff000);
}


//kernel_thread执行function(func_arg)
static void kernel_thread(thread_func* function, void* func_arg)
{
    //开中断
    intr_enable();
    function(func_arg);
}

//初始化线程基本信息
void init_thread(struct task_struct* pthread, char* name, int pri)
{
    memset(pthread, 0, sizeof(*pthread));  

    //self_kstack是线程在内核态下使用的栈顶地址
    pthread->self_kstack   = (uint32_t*)((uint32_t)pthread + PG_SIZE);

    //主线程应处于一直运行的状态
    if (pthread == main_thread) {
        pthread->status = TASK_RUNNING; 
    } else {
        pthread->status = TASK_READY;
    }

    strcpy(pthread->name, name);

    pthread->priority      = pri;
    pthread->ticks         = pri;
    pthread->elapsed_ticks = 0; pthread->pgdir         = NULL;
    pthread->stack_magic   = 0x19991005;
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

//新建线程并加入队列
struct task_struct* thread_start(char* name, int pri, thread_func function, void* func_arg)
{
    struct task_struct* thread = get_kernel_pages(1);

    init_thread(thread, name, pri);
    thread_create(thread, function, func_arg);

    ASSERT(!node_find(&thread_ready_list, &thread->general_tag));
    list_append(&thread_ready_list, &thread->general_tag);

    ASSERT(!node_find(&thread_all_list, &thread->all_list_tag));
    list_append(&thread_all_list, &thread->all_list_tag);


    //asm volatile ("movl %0, %%esp; pop %%ebp; \
                   pop %%ebx; pop %%edi; pop %%esi; ret" : : "g" (thread->self_kstack) : "memory");

    return thread;
}

void schedule(void)
{
    //保证是关中断状态下调用的schedule
    ASSERT(get_intr_status() == INTR_OFF);

    struct task_struct* curr_thread = running_thread();
    if (curr_thread->status == TASK_RUNNING) {
        ASSERT(!node_find(&thread_ready_list, &curr_thread->general_tag));
        list_append(&thread_ready_list, &curr_thread->general_tag);
        curr_thread->ticks = curr_thread->priority;
        curr_thread->status = TASK_READY;
    }

    ASSERT(!list_empty(&thread_ready_list));
    thread_tag = NULL;
    thread_tag = list_pop(&thread_ready_list);
    //通过tag获取对应PCB的起始地址
    //struct task_struct*  next_thread = node2entry(struct task_struct, general_tag, thread_tag);
    struct task_struct* next_thread = (struct task_struct*) ((uint32_t)thread_tag - (uint32_t)(&((struct task_struct*)0x0)->general_tag));
    next_thread->status = TASK_RUNNING;
    switch_to(curr_thread, next_thread);
}

static void make_main_thread(void)
{
    main_thread = running_thread();
    init_thread(main_thread, "main", 31);

    ASSERT(!node_find(&thread_all_list, &main_thread->all_list_tag));
    list_append(&thread_all_list, &main_thread->all_list_tag);
}

void init_thread_env(void)
{
    put_str("[thread env]: init start\n");
    list_init(&thread_ready_list);
    list_init(&thread_all_list);
    make_main_thread();

    put_str("[thread env]: env init end\n");
}
