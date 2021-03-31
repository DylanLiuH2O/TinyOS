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

//门描述符,结构体中地址向下增大
struct gate_desc {
    uint16_t func_offset_low_word;  //中断处理程序偏移量的低16位
    uint16_t selector;              //中断处理程序目标段代码选择子
    uint8_t  dcount;                //门描述符高32位未使用的部分
    uint8_t  attribute;             //属性位8~15位
    uint16_t func_offset_high_word; //中断处理程序偏移量的低16位
};

extern intr_handler intr_entry_table[IDT_DESC_CNT]; //中断处理程序入口点数组
static struct gate_desc idt[IDT_DESC_CNT];          //中断描述符表,后面用lidt指令载入

static void set_idt_desc(struct gate_desc* p_gdesc, uint8_t attr, intr_handler func_addr);
static void idt_desc_init(void);

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
    put_str("    idt_desc_init done\n");
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
}

//初始化中断描述符表
void idt_init(void) {
    put_str("idt_init start\n");
    idt_desc_init();
    pic_init();

    //将idt的地址转换为64位,并用内联汇编载入idt
    uint64_t idt_operand = ((sizeof(idt) - 1) | ( (uint64_t)( (uint32_t)idt << 16 ) ));
    asm volatile ("lidt %0" : : "m"(idt_operand) );
    put_str("idt_init done\n");
}
