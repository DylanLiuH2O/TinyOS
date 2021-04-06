#ifndef __MEMORY_H
#define __MEMORY_H
#include "stdint.h"
#include "bitmap.h"

struct vaddr_pool {
    struct bitmap vpbitmap;
    uint32_t vaddr_start;

};

struct pool {
    struct bitmap pbitmap;
    uint32_t phy_addr_start;
    uint32_t size;
};

void mem_init(void);

#endif
