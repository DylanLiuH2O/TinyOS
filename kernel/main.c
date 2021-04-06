#include "print.h"
#include "init.h"
#include "debug.h"
#include "stdint.h"
#include "string.h"
#include "bitmap.h"
#include "memory.h"
#define DEBUG
int main(void) {
    put_str("I am kernel\n");
    init_all();
    
    void* addr = get_kernel_pages(3);
    put_str("\nget_kernel_page start vaddr is: ");
    put_int((uint32_t)addr);
    put_char('\n');

    while(1);
    return 0;
}
