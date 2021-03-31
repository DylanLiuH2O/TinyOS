#ifndef _LIB_KERNEL_PRINT_H
#define _LIB_KERNEL_PRINT_H
#include "stdint.h"
void put_char(uint8_t char_ascii);
void put_str(char* msg);
void put_int(uint32_t num);
#endif
