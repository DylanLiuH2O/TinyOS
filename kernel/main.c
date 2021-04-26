#include "lib/stdint.h"
#include "lib/string.h"
#include "lib/kernel/print.h"
#include "lib/kernel/bitmap.h"
#include "kernel/init.h"
#include "kernel/interrupt.h"
#include "kernel/debug.h"
#include "kernel/memory.h"
#include "thread/thread.h"
#include "device/console.h"

#define DEBUG

void k_thread_a(void* arg);
void k_thread_b(void* arg);

int main(void) {
    put_str("I am kernel\n");
    init_all();
    
    thread_start("k_thread_a", 31, k_thread_a, "argA ");
    thread_start("k_thread_b", 31, k_thread_b, "argB ");

    intr_enable();
    while(1) {
        console_put_str("Main ");
    }

    while(1);
    return 0;
}

void k_thread_a(void* arg)
{
    char* para = (char*)arg;
    while (1) {
        console_put_str(para);
    }
}

void k_thread_b(void* arg)
{
    char* para = (char*)arg;
    while (1) {
        console_put_str(para);
    }
}