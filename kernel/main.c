#include "print.h"
#include "init.h"
#include "debug.h"
#include "stdint.h"
#include "string.h"
#include "bitmap.h"
#include "memory.h"
#include "thread.h"
#define DEBUG

void k_thread_a(void* arg);

int main(void) {
    put_str("I am kernel\n");
    init_all();
    
    thread_start("k_thread_a", 31, k_thread_a, "argA ");

    while(1);
    return 0;
}

void k_thread_a(void* arg)
{
    char* para = (char*)arg;
    while (1) {
        put_str(para);    
    }
}
