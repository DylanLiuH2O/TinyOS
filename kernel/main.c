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
#include "device/keyboard.h"

#define DEBUG

void k_thread_a(void* arg);
void k_thread_b(void* arg);

int main(void) {
    put_str("I am kernel\n");
    init_all();
    
    thread_start("consumer_a", 31, k_thread_a, "A_");
    thread_start("consumer_b", 31, k_thread_b, "B_");

    intr_enable();
    while(1) {
        //console_put_str("Main ");
    }

    while(1);
    return 0;
}

void k_thread_a(void* arg)
{
    char* para = (char*)arg;
    while (1) {
        char ch = ioqueue_getchar(&kb_buf);
        console_put_str(para);
        console_put_char(ch);
        console_put_char(' ');
    }
}

void k_thread_b(void* arg)
{
    char* para = (char*)arg;
    while (1) {
        char ch = ioqueue_getchar(&kb_buf);
        console_put_str(para);
        console_put_char(ch);
        console_put_char(' ');
    }
}