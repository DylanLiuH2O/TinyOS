#include "device/console.h"
#include "lib/kernel/print.h"
#include "lib/stdint.h"
#include "thread/sync.h"
#include "thread/thread.h"

static struct lock console_lock;

void console_init(void)
{
    lock_init(&console_lock);
}

void console_acquire(void)
{
    lock_acquire(&console_lock);
}

void console_release(void)
{
    lock_release(&console_lock);
}

void console_put_char(uint8_t char_ascii)
{
    lock_acquire(&console_lock);
    put_char(char_ascii);
    lock_release(&console_lock);
}

void console_put_int(uint32_t num)
{
    lock_acquire(&console_lock);
    put_int(num);
    lock_release(&console_lock);
}

void console_put_str(char* str)
{
    lock_acquire(&console_lock);
    put_str(str);
    lock_release(&console_lock);
}