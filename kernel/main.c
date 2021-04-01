#include "print.h"
#include "init.h"
#include "debug.h"
#define DEBUG
int main(void) {
    put_str("I am kernel\n");
    init_all();
    //asm volatile("sti");    open interrupt
    ASSERT(1 == 2);
    while(1);
}
