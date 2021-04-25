#include "lib/kernel/print.h"
#include "lib/kernel/io.h"
#include "lib/stdint.h"
#include "kernel/interrupt.h"
#include "kernel/debug.h"
#include "device/timer.h"
#include "thread/thread.h"

#define IRQ0_FREQUENCY   100
#define INPUT_FREQUENCY  1193180
#define COUNTER0_VALUE   INPUT_FREQUENCY / IRQ0_FREQUENCY
#define COUNTER0_PORT    0x40
#define COUNTER0_NO      0
#define COUNTER_MODE     2
#define READ_WRITE_LATCH 3
#define PIT_CONTROL_PORT 0x43

uint32_t ticks;

static void set_frequency(uint8_t counter_port, uint8_t counter_no, \
                          uint8_t rwl, uint8_t counter_mode, \
                          uint16_t counter_value) {
    //
    outb(PIT_CONTROL_PORT, (uint8_t)(counter_no << 6 | rwl << 4 | counter_mode << 1));
    outb(counter_port, (uint8_t)counter_value);
    outb(counter_port, (uint8_t)counter_value >> 8);
}

static void intr_timer_handler(void)
{
    struct task_struct* curr_thread = running_thread();

    //通过判断标记部分是否被覆盖得知栈是否已满
    ASSERT(curr_thread->stack_magic == 0x19991005);

    curr_thread->elapsed_ticks++;
    ticks++;

    if (curr_thread->ticks == 0) {
        schedule(); //时间片为0,调度下一个线程
    } else {
        curr_thread->ticks--; 
    }
}

void timer_init(void)
{
    put_str("[timer]: init start\n");
    set_frequency(COUNTER0_PORT, COUNTER0_NO, READ_WRITE_LATCH, COUNTER_MODE, COUNTER0_VALUE);
    register_handler(0x20, intr_timer_handler);
    put_str("[timer]: init done\n");
}
