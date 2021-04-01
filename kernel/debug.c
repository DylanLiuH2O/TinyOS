#include "debug.h"
#include "print.h"
#include "interrupt.h"

void panic_spin(char* filename, int line, const char* func, const char* condition) {

    //关中断保证原子性
    intr_disable();

    put_str("\n[error]\n");

    put_str("filename:");
    put_str(filename);
    put_str("\n");

    put_str("line:0x");
    put_int(line);
    put_str("\n");

    put_str("function:");
    put_str((char*)func);
    put_str("\n");

    put_str("condition:");
    put_str((char*)condition);
    put_str("\n");

    while(1);
}
