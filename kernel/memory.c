#include "memory.h"
#include "stdint.h"
#include "print.h"
#include "string.h"

#define PG_SIZE 4096                //页大小为4K
#define MEM_BITMAP_BASE 0xc009a000  //内存管理位图在0xc009a000~0xc009afff
#define KERNEL_HEAP_BASE 0xc0100000

struct pool kernel_pool;             //内核内存池
struct pool user_pool;               //用户内存池
struct vaddr_pool kernel_vaddr_pool; //内核虚拟地址池,用于判断该虚拟地址是否被使用


static void mem_pool_init(uint32_t total_mem) 
{
    put_str("[memory pool]: init start\n");
    uint32_t used_mem   = PG_SIZE * 256 + 0x100000;
    uint32_t free_mem   = total_mem - used_mem;
    uint16_t free_pages = free_mem / PG_SIZE;

    uint16_t k_free_pages = free_pages / 2;
    uint16_t u_free_pages = free_pages - k_free_pages;

    uint16_t k_bitmap_size = k_free_pages / 8;
    uint16_t u_bitmap_size = u_free_pages / 8;

    //初始化内核内存池
    put_str("  [kernel memory pool]: init start\n");
    kernel_pool.pbitmap.bits      = (void*)MEM_BITMAP_BASE;
    kernel_pool.phy_addr_start    = used_mem;
    kernel_pool.size              = k_bitmap_size;
    
    bitmap_init(&kernel_pool.pbitmap, k_free_pages);

    put_str("    kernel_pool.pbitmap.bits: ");
    put_int((uint32_t)kernel_pool.pbitmap.bits);
    put_char('\n');
    put_str("    kernel_pool.pbitmap.bytes_len: ");
    put_int((uint32_t)kernel_pool.pbitmap.bytes_len);
    put_char('\n');
    put_str("    kernel_pool.phy_addr_start: ");
    put_int((uint32_t)kernel_pool.phy_addr_start);
    put_char('\n');
    put_str("    kernel_pool.size: ");
    put_int((uint32_t)kernel_pool.size);
    put_char('\n');


    //初始化用户内存池
    put_str("  [user memory pool]: init start\n");
    user_pool.pbitmap.bits      = (void*)(MEM_BITMAP_BASE + k_bitmap_size);
    user_pool.size              = u_bitmap_size;
    user_pool.phy_addr_start    = used_mem + k_free_pages * PG_SIZE;

    bitmap_init(&user_pool.pbitmap, u_free_pages);

    put_str("    user_pool.pbitmap.bits: ");
    put_int((uint32_t)user_pool.pbitmap.bits);
    put_char('\n');
    put_str("    user_pool.pbitmap.bytes_len: ");
    put_int((uint32_t)user_pool.pbitmap.bytes_len);
    put_char('\n');
    put_str("    user_pool.phy_addr_start: ");
    put_int((uint32_t)user_pool.phy_addr_start);
    put_char('\n');
    put_str("    user_pool.size: ");
    put_int((uint32_t)user_pool.size);
    put_char('\n');
    put_str("  [user memory pool]: init done\n");


    //初始化内核虚拟地址池
    put_str("  [kernel vaddr pool]: init start\n");
    kernel_vaddr_pool.vpbitmap.bits      =
            (void*)(MEM_BITMAP_BASE + k_bitmap_size + u_bitmap_size);
    kernel_vaddr_pool.vaddr_start        = KERNEL_HEAP_BASE;

    bitmap_init(&kernel_vaddr_pool.vpbitmap, k_free_pages);

    put_str("    kernel_vaddr_pool.vpbitmap.bits: ");
    put_int((uint32_t)kernel_vaddr_pool.vpbitmap.bits);
    put_char('\n');
    put_str("    kernel_vaddr_pool.vpbitmap.bytes_len: ");
    put_int((uint32_t)kernel_vaddr_pool.vpbitmap.bytes_len);
    put_char('\n');
    put_str("    kernel_vaddr_pool.vaddr_start: ");
    put_int((uint32_t)kernel_vaddr_pool.vaddr_start);
    put_char('\n');
    put_str("  [kernel vaddr pool]: init done\n");



    put_str("[memory pool]: init done\n");
}

void mem_init(void) 
{
    uint32_t mem_size = (*(uint32_t*)(0xb00));
    mem_pool_init(mem_size);
}



