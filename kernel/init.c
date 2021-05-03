#include "kernel/init.h"
#include "kernel/interrupt.h"
#include "kernel/memory.h"
#include "lib/kernel/print.h"
#include "device/timer.h"
#include "thread/thread.h"
#include "device/console.h"
#include "device/keyboard.h"

void init_all(void) {
    put_str("init_all\n");
    idt_init();
    mem_init();
    init_thread_env();
    timer_init();
    console_init();
    keyboard_init();
}
