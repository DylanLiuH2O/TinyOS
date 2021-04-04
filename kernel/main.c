#include "print.h"
#include "init.h"
#include "debug.h"
#include "stdint.h"
#include "string.h"
#include "bitmap.h"
#define DEBUG
int main(void) {
    put_str("I am kernel\n");
    init_all();
    //asm volatile("sti");    open interrupt
    //ASSERT(1 == 2);

    struct bitmap bm; 

    bm.bits = (uint8_t*)0xc009a000;
    bitmap_init(&bm, 5);
    bm.bits[0] = 0xff;

    put_str("test bitmap:\n");
    uint32_t index = bitmap_search(&bm, 10);
    put_str("  expect: 8");
    put_str("  actual: ");
    put_int(index);
    put_char('\n');

    put_str("test bitmap_set:\n");
    for (int i = 0; i < 10; i++) {
        bitmap_set(&bm, index+i, 1);
    }
    index = bitmap_search(&bm, 10);
    put_str("  expect: 12");
    put_str("  actual: ");
    put_int(index);
    put_char('\n');

    while(1);
}
