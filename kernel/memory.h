#ifndef __KERNEL_MEMORY_H
#define __KERNEL_MEMORY_H

#include "lib/stdint.h"
#include "lib/kernel/bitmap.h"

#define PG_P_1  1   //页表存在位
#define PG_P_0  0   //页表存在位
#define PG_RW_R 0   //页表读写属性,读/执行
#define PG_RW_W 2   //页表读写属性,读/写/执行
#define PG_US_S 0   //系统级属性
#define PG_US_U 4   //用户级属性

enum POOL_FLAGS {
    PF_KERNEL = 1,
    PF_USER   = 2
};

struct vaddr_pool {
    struct bitmap vpbitmap;
    uint32_t vaddr_start;

};

struct pool {
    struct bitmap pbitmap;
    uint32_t phy_addr_start;
    uint32_t size;
};

void* malloc_page(enum POOL_FLAGS pf, uint32_t pg_cnt);
void* get_kernel_pages(uint32_t pg_cnt);
void mem_init(void);

#endif
