#ifndef __LIB_STRING_H
#define __LIB_STRING_H

#include "lib/stdint.h"

void memset(void* dst, uint8_t value, uint32_t size);
void memcpy(void* dst, const void* src, uint32_t size);
int8_t memcmp(const void* dst, const void* src, uint32_t size);
char* strcpy(char* dst, const char* src);
uint32_t strlen(const char* str);
int8_t strcmp(const char* a, const char* b);
char* strchr(const char* str, const uint8_t ch);
char* strchr_r(const char* str, const uint8_t ch);
char* strcat(char* dst, const char* src);
uint32_t str_count(const char* str, const uint8_t ch);

#endif
