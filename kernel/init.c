#include "init.h"
#include "../lib/kernel/print.h"
#include "interrupt.h"

void init_all(void) {
    put_str("init_all\n");
    idt_init();
}
