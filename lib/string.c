#include "lib/stdint.h"
#include "lib/string.h"
#include "kernel/debug.h"

#define NULL 0

void memset(void* dst, uint8_t value, uint32_t size) {
    ASSERT(dst != NULL);
    uint8_t* dst_ = (uint8_t*)dst;
    while (size != 0) {
        *dst_ = value; 
        dst_++;
        size--;
    }
}

void memcpy(void* dst, const void* src, uint32_t size) {
    ASSERT(dst != NULL && src != NULL);
    uint8_t* dst_ = (uint8_t*)dst;
    uint8_t* src_ = (uint8_t*)src;
    while (size != 0) {
        *dst_ = *src_;
        dst_++;
        src_++;
        size--;
    }
}

int8_t memcmp(const void* dst, const void* src, uint32_t size) {
    ASSERT(dst != NULL && src != NULL);
    uint8_t* dst_ = (uint8_t*)dst;
    uint8_t* src_ = (uint8_t*)src;
    while (size != 0) {
        if (*dst_ != *src_) {
            return *dst_ > *src_ ? 1 : -1;
        }
        dst_++;
        src_++;
        size--;
    }

    return 0;
}

char* strcpy(char* dst, const char* src) {
    ASSERT(dst != NULL && src != NULL);
    char* cur = dst;
    while (*cur != '\0') {
        *cur = *src; 
        cur++;
        src++;
    }

    return dst;
}

uint32_t strlen(const char* str) {
    uint32_t len = 0;
    ASSERT(str != NULL);
    while (*str != '\0') {
        str++;
        len++;
    }
    return len;
}

int8_t strcmp(const char* a, const char* b) {
    ASSERT(a != NULL && b != NULL);
    while (*a != '\0' && *b != '\0' && *a == *b) {
        a++;
        b++;
    }
    return *a < *b ? -1 : *a > *b;
}

char* strchr(const char* str, const uint8_t ch) {
    ASSERT(str != NULL);
    while (*str != '\0' && *str != ch) {
        str++; 
    }
    return (char*)str;
}

char* strchr_r(const char* str, const uint8_t ch) {
    ASSERT(str != NULL);
    while (*str != '\0') {
        str++; 
    }
    return (char*)str;
}

char* strcat(char* dst, const char* src) {
    ASSERT(dst != NULL && src != NULL);
    char* tmp = dst;
    while (*dst != '\0') {
        dst++; 
    }
    while (*src != '\0') {
        *dst = *src; 
        dst++;
        src++;
    }
    *dst = '\0';
    return tmp;
}

uint32_t str_count(const char* str, const uint8_t ch) {
    ASSERT(str != NULL);
    uint32_t cnt = 0;
    while (*str != '\0') {
        cnt += (*str == ch) ? 1 : 0;
        str++;
    }
    return cnt;
}
