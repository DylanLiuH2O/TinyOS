#ifndef __BITMAP_H
#define __BITMAP_H
#include "stdint.h"
#define _BOOL uint8_t
struct bitmap {
    uint32_t bytes_len;
    uint8_t* bits;
};

//初始化位图
void bitmap_init(struct bitmap* bm, uint32_t bytes_len);

//判断位是否被使用
_BOOL bitmap_isused(const struct bitmap* bm, uint32_t bit_index);

//查找是否有cnt个位可用的位
uint32_t bitmap_search(const struct bitmap* bm, uint32_t cnt);

//设置cnt个可用的位
void bitmap_set(struct bitmap* bm, uint32_t bit_index, int8_t value);




#endif
