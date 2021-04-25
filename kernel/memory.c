#include "kernel/memory.h"
#include "kernel/debug.h"
#include "lib/stdint.h"
#include "lib/kernel/print.h"
#include "lib/string.h"

#define PG_SIZE 4096                //页大小为4K
#define MEM_BITMAP_BASE 0xc009a000  //内存管理位图在0xc009a000~0xc009afff
#define KERNEL_HEAP_BASE 0xc0100000

#define PDE_IDX(addr) ((addr & 0xffc00000) >> 22)
#define PTE_IDX(addr) ((addr & 0x003ff000) >> 12)
#define GET_PDE_PTR(vaddr) ((uint32_t*)((0xfffff000) + \
                           (PDE_IDX(vaddr) * 4)))
#define GET_PTE_PTR(vaddr) ((uint32_t*)(0xffc00000 + \
                           ((vaddr & 0xffc00000) >> 10) + \
                           (PTE_IDX(vaddr) * 4)))

#define NULL 0x0

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

//在指定虚拟地址池中获取pg_cnt个可用的虚拟地址
static void* vaddr_get(enum POOL_FLAGS pf, uint32_t pg_cnt)
{
    uint32_t vaddr = 0x0;
    uint32_t bit_index_start = -1;
    if (pf == PF_KERNEL) {
        bit_index_start = bitmap_search(&kernel_vaddr_pool.vpbitmap, pg_cnt); 
        if (bit_index_start == -1) {
            return NULL; 
        }
        for (int i = 0; i < pg_cnt; i++) {
            bitmap_set(&kernel_vaddr_pool.vpbitmap, bit_index_start + i, 1); 
        }
        vaddr = kernel_vaddr_pool.vaddr_start + bit_index_start * PG_SIZE;
    } else {
        //user
    }
    return (void*)vaddr;
}

/*
uint32_t* pte_ptr(uint32_t vaddr)
{
    uint32_t* pte = (uint32_t*)(0xffc00000 + ((vaddr & 0xffc00000) >> 10) + (PTE_IDX(vaddr) << 2));
    return pte;
}

uint32_t* pde_ptr(uint32_t vaddr)
{
    uint32_t* pde = (uint32_t*)((0xfffff000) + PDE_IDX(vaddr) * 4);
    return pde;
}
*/

//在指定内存池中获取1个空闲物理页的物理地址
static void* palloc(struct pool* mem_pool)
{
    uint32_t phy_page_addr = 0x0;
    uint32_t bit_index = bitmap_search(&mem_pool->pbitmap, 1);
    if (bit_index == -1) {
        return NULL; 
    }
    bitmap_set(&mem_pool->pbitmap, bit_index, 1);
    phy_page_addr = mem_pool->phy_addr_start + bit_index * PG_SIZE;

    return (void*)phy_page_addr;
}

//在页表中建立虚拟地址与物理页的映射
static void pte_add_page(void* vaddr, void* phy_page_addr)
{
    uint32_t* pde = GET_PDE_PTR((uint32_t)vaddr);
    uint32_t* pte = GET_PTE_PTR((uint32_t)vaddr);

    //通过页目录项判断对应页表是否已经存在
    if (*pde & 0x0000001) {
        ASSERT(!(*pte & 0x00000001));    
        //建立页表项与物理地址的映射,US = 1, RW = 1, P = 1
        *pte = ((uint32_t)phy_page_addr | PG_US_S | PG_RW_R | PG_P_1);
    } else {
        //页目录项对应的页表不存在,应先创建页表
        uint32_t pde_phy_addr = (uint32_t)palloc(&kernel_pool);

        *pde = (pde_phy_addr | PG_US_S | PG_RW_R | PG_P_1);

        //将新分配页表的数据清零
        memset((void*)((uint32_t)0xffc00000 + (((uint32_t)vaddr & 0xffc00000) >> 10) + 0), 0, PG_SIZE);

        //建立页表项与物理地址的映射,US = 1, RW = 1, P = 1
        *pte = ((uint32_t)phy_page_addr | PG_US_S | PG_RW_R | PG_P_1);
    }
}

//在指定内存池中分配pg_cnt个物理页
void* malloc_page(enum POOL_FLAGS pf, uint32_t pg_cnt)
{
    //1.获取可用虚拟地址
    //2.获取可用物理地址
    //3.建立映射

    void* free_vaddr_start  =  vaddr_get(pf, pg_cnt);
    if (free_vaddr_start == NULL) {
        return NULL; 
    }

    struct pool* mem_pool = pf == PF_KERNEL ? &kernel_pool : &user_pool;
    for (int i = 0; i < pg_cnt; i++) {
        void* free_page_phyaddr = palloc(mem_pool);
        //失败后要回滚已映射好的物理页
        if (free_page_phyaddr == NULL) {
            return NULL; 
        }
        pte_add_page((free_vaddr_start + i * PG_SIZE), free_page_phyaddr);
    }
    
    return free_vaddr_start;
}

//向内核内存池申请pg_cnt页物理内存
void* get_kernel_pages(uint32_t pg_cnt)
{
    void* vaddr = malloc_page(PF_KERNEL, pg_cnt);
    if (vaddr != NULL) {
        memset(vaddr, 0, pg_cnt*PG_SIZE);
    }
    return vaddr;
}

void mem_init(void) 
{
    //从地址0xb00处获取实际内存容量(使用BIOS中断获取的)
    uint32_t mem_size = (*(uint32_t*)(0xb00));
    mem_pool_init(mem_size);
}