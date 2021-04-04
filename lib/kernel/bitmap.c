#include "bitmap.h"
#include "string.h"
#include "stdint.h"
#include "debug.h"
#include "print.h"

void bitmap_init(struct bitmap* bm, uint32_t bytes_len) {
    bm->bytes_len = bytes_len;
    memset(bm->bits, 0, bm->bytes_len);
}

_BOOL bitmap_isused(const struct bitmap* bm, uint32_t bit_index) {
    uint32_t byte_index = bit_index / 8;
    uint8_t mask = 0x1 << (bit_index % 8);

    //put_int(bm->bits[byte_index] & mask);
    //put_char('\n');
    return (bm->bits[byte_index] & mask) != 0;
}

uint32_t bitmap_search(const struct bitmap* bm, uint32_t cnt) {
    uint32_t left_index = -1;
    uint32_t right_index = -1;

    //从头开始寻找第一个可用的位
    for (int i = 0; i < bm->bytes_len; i++) {
        if (bm->bits[i] != 0xff) {
            left_index = i * 8; 
            right_index = left_index;
            break;
        }
    }
    ASSERT(left_index != -1);

    //put_str("size: ");
    //put_int(bm->bytes_len * 8);
    //put_char('\n');
    //双指针查找连续的空闲位
    while (left_index < bm->bytes_len * 8) {
        //put_str("left_index: ");
        //put_int(left_index);
        //put_str(" right_index: ");
        //put_int(right_index);
        //put_char('\n');
        if (bitmap_isused(bm, right_index) == 0) {
            if (right_index - left_index + 1 == cnt) {
                return left_index; 
            }
        } else {
            left_index = right_index + 1; 
        }
        right_index++;
    }

    return -1;
}

void bitmap_set(struct bitmap* bm, uint32_t bit_index, int8_t value) {
    ASSERT(value == 0 || value == 1);
    uint32_t byte_index = bit_index / 8;
    uint8_t mask = 0x1 << (bit_index % 8);
    
    //put_str("bitmap_set index: ");
    //put_int(bit_index);
    //put_char('\n');
    if (value == 1) {
        bm->bits[byte_index] |= mask;
    } else {
        bm->bits[byte_index] &= (~mask);
    }

}
