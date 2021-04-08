#ifndef __THREAD_H
#define __THREAD_H
#include "stdint.h"
#include "list.h"


//自定义通用函数类型
typedef void thread_func(void*);

//任务状态
enum task_status {
    TASK_RUNNING,
    TASK_READY,
    TASK_BLOCKED,
    TASK_WAITING,
    TASK_HANGING,
    TASK_DIED
};

struct intr_stack {
    uint32_t vec_no;
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp_dummy;

    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;

    //从低特权级到高特权级时压入
    uint32_t err_code;
    void (*eip) (void);
    uint32_t cs;
    uint32_t eflags;
    void*    esp;
    uint32_t ss;
};

//线程栈
struct thread_stack {
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edi;
    uint32_t esi;

    void (*eip) (thread_func* func, void* func_arg);

    void (*unused_retaddr);
    thread_func* function;
    void* func_arg;
};

struct task_struct {
    uint32_t* self_kstack;          //线程内核栈
    enum      task_status status;
    char      name[16];             //
    uint32_t  priority;             //优先级

    uint8_t  ticks;
    uint32_t elapsed_ticks;         //任务执行的时钟数

    struct list_node general_tag;

    struct list_node all_list_tag;


    uint32_t* pgdir;            //进程自己页表的虚拟地址
    uint32_t  stack_magic;      //用于检测栈是否已满
};

struct task_struct* running_thread(void);
struct task_struct* thread_start(char* name, int pri, thread_func function, void* func_arg);
void schedule(void);
void init_thread_env(void);

#endif
