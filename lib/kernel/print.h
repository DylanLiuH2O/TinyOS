#ifndef __LIB_KERNEL_PRINT_H
#define __LIB_KERNEL_PRINT_H

#include "lib/stdint.h"

void put_char(uint8_t char_ascii);
void put_str(char* msg);
void put_int(uint32_t num);
void set_cursor(uint32_t pos);

#endif
