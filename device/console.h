#ifndef __DEVICE_CONSOLE_H
#define __DEVICE_CONSOLE_H

#include "lib/stdint.h"

void console_init(void);
void console_acquire(void);
void console_release(void);
void console_put_char(uint8_t char_ascii);
void console_put_int(uint32_t num);
void console_put_str(char* str);

#endif