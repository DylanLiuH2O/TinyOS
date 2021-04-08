#include "print.h"      //汇编提供的打印功能
#include "stdint.h"     //类型封装
#include "interrupt.h"  //中断相关
#include "global.h"
#include "io.h"

#define IDT_DESC_CNT 0x21

#define PIC_M_CTRL 0x20
#define PIC_M_DATA 0x21
#define PIC_S_CTRL 0xa0
#define PIC_S_DATA 0xa1

#define EFLAGS_IF 0x00000200
#define GET_EFLAGS(EFLAG_VAR) asm volatile("pushfl; popl %0" : "=g" (EFLAG_VAR))

//门描述符,结构体中地址向下增大
struct gate_desc {
    uint16_t func_offset_low_word;  //中断处理程序偏移量的低16位
    uint16_t selector;              //中断处理程序目标段代码选择子
    uint8_t  dcount;                //门描述符高32位未使用的部分
    uint8_t  attribute;             //属性位8~15位
    uint16_t func_offset_high_word; //中断处理程序偏移量的低16位
};

extern intr_handler     intr_entry_table[IDT_DESC_CNT]; //中断处理程序入口点数组(在kernel.S中)
char*                   intr_name[IDT_DESC_CNT];        //中断对应的异常名
intr_handler            idt_table[IDT_DESC_CNT];        //C编写的更进一步的中断处理程序
static struct gate_desc idt[IDT_DESC_CNT];    //中断描述符表,后面用lidt指令载入


static void set_idt_desc(struct gate_desc* p_gdesc, uint8_t attr, intr_handler func_addr);
static void idt_desc_init(void);
static void pic_init(void);
static void general_intr_handler(uint8_t vec_nr);
static void exception_init(void);

void idt_init(void);

/*
 31                        16 15 14 13   12   11     8  7   6   5  4         0
 ------------------------------------------------------------------------------
 |  intr_func_offset(31~16)  | P | DPL | S(0) | TYPE() | 0 | 0 | 0 | NOT USED |   High 32-bits
 ------------------------------------------------------------------------------
 |   func_offset_high_word   |        attribute        |        dcount        |

 31                        16 15                                             0
 ------------------------------------------------------------------------------
 |        func_selector      |             intr_func_offset(15~0)             |   Low 32-bits
 ------------------------------------------------------------------------------
 |         selector          |               func_offset_low_word             |

                            Interrupt Gate Descriptor
*/
//设置中断描述符
static void set_idt_desc(struct gate_desc* p_gdesc, uint8_t attr, intr_handler func_addr) {
    p_gdesc->func_offset_low_word = (uint32_t)func_addr & 0x0000FFFF;
    p_gdesc->selector = SELECTOR_K_CODE;
    p_gdesc->dcount = 0;
    p_gdesc->attribute = attr;
    p_gdesc->func_offset_high_word = ((uint32_t)func_addr & 0xFFFF0000) >> 16;
}

//初始化所有中断描述符
static void idt_desc_init(void) {
    for (int i = 0; i < IDT_DESC_CNT; i++) {
        set_idt_desc(&idt[i], IDT_DESC_ATTR_DPL0, intr_entry_table[i]);    
    }
    put_str("[idt_desc]: init done\n");
}

//初始化可编程中断逻辑器件8259A
static void pic_init(void) {
    /*initialize master chip*/
    outb(PIC_M_CTRL, 0x11);
    outb(PIC_M_DATA, 0x20);

    outb(PIC_M_DATA, 0x04);
    outb(PIC_M_DATA, 0x01);

    /*initialize slave chip*/
    outb(PIC_S_CTRL, 0x11);
    outb(PIC_S_DATA, 0x28);

    outb(PIC_S_DATA, 0x02);
    outb(PIC_S_DATA, 0x01);

    /*open IR0*/
    outb(PIC_M_DATA, 0xfe);
    outb(PIC_S_DATA, 0xff);

    put_str("[pic]: init done\n");
}


//通用中断处理函数
static void general_intr_handler(uint8_t vec_nr) {
    if (vec_nr == 0x27 || vec_nr == 0x2f) {
        return; 
    }

    set_cursor(0);  //该函数在print.S中
    for (int i = 0; i < 320; i++) {
        put_char(' '); 
    }
    set_cursor(0);
    put_str("!!!!!!!!    exception message begin    !!!!!!!!\n");
    set_cursor(88);
    put_str(intr_name[vec_nr]);
    if (vec_nr == 14) {
        int page_fault_vaddr = 0; 
        asm volatile ("movl %%cr2, %0" : "=r" (page_fault_vaddr));
        put_str("\npage fault addr is : ");
        put_int(page_fault_vaddr);
        put_char('\n');
    }
    put_str("\n!!!!!!!!    exception message end    !!!!!!!!\n");
    while(1);

    /*
    put_str("int vector: 0x");
    put_int(vec_nr);
    put_char('\n');
    put_str(intr_name[vec_nr]);
    put_char('\n');
    */
}

static void exception_init(void) {
    for (int i = 0; i < IDT_DESC_CNT; i++) {
        idt_table[i] = general_intr_handler;
        intr_name[i] = "unknown";
    }
    intr_name[0] = "#DE Divide Error";
    intr_name[1] = "#DB Debug";
    intr_name[2] = "NMI Interrupt";
    intr_name[3] = "#BP Breakpoint Exception";
    intr_name[4] = "#OF Overflow Exception";
    intr_name[5] = "#BR BOUND Range Exceeded Exception";
    intr_name[6] = "#UD Invalid Opcode Exception";
    intr_name[7] = "#NM Device Not Available Exception";
    intr_name[8] = "#DF Double Fault Exception";
    intr_name[9] = "Coprocessor Segment Overrun";
    intr_name[10] = "#TS Invalid TSS Exception";
    intr_name[11] = "#NP Segment Not Present";
    intr_name[12] = "#SS Stack Fault Exception";
    intr_name[13] = "#GP Genernal Protection Exception";
    intr_name[14] = "#PF Page-Fault Exception";
    //intr_name[15] = ""; reserve
    intr_name[16] = "#MF x87 FPU Floating-Point Error";
    intr_name[17] = "#AC Alignment Check Exception";
    intr_name[18] = "#MC Machine-Check Exception";
    intr_name[19] = "#XF SIMD Floating-Point Exception";

}

//注册中断处理程序(本质上就是将函数地址放入中断处理程序表)
void register_handler(uint8_t vector_no, intr_handler function)
{
    idt_table[vector_no] = function;
}

//初始化中断描述符表
void idt_init(void) {
    put_str("[idt]: init start\n");
    idt_desc_init();
    exception_init();
    pic_init();

    //将idt的地址转换为64位,并用内联汇编载入idt
    uint64_t idt_operand = ((sizeof(idt) - 1) | ( (uint64_t)( (uint32_t)idt << 16 ) ));
    asm volatile ("lidt %0" : : "m"(idt_operand) );
    put_str("[idt]: init done\n");
}

enum intr_status get_intr_status(void) {
    uint32_t eflags = 0;
    GET_EFLAGS(eflags);

    return (eflags & INTR_ON) != 0 ? INTR_ON : INTR_OFF;
}

enum intr_status set_intr_status(enum intr_status status) {
    return (status & INTR_ON) != 0 ? intr_enable() : intr_disable();
}

//开中断,返回开中断前的状态
enum intr_status intr_enable(void) {
    enum intr_status old_status = get_intr_status();
    if (old_status == INTR_OFF) {
        asm volatile ("sti"); //IF位置1
    }

    return old_status;
}

//关中断,返回关中断前的状态
enum intr_status intr_disable(void) {
    enum intr_status old_status = get_intr_status();
    if (old_status == INTR_ON) {
        asm volatile ("cli" : : : "memory"); //IF位置0
    }

    return old_status;
}
