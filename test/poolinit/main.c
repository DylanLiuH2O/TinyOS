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

    mem_init(); 

    while(1);
}
