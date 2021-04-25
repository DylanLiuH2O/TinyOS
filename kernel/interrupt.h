#ifndef __KERNEL_INTERRUPT_H
#define __KERNEL_INTERRUPT_H

#include "lib/stdint.h"

typedef void* intr_handler;

enum intr_status {
    INTR_OFF,       //关中断
    INTR_ON         //开中断
};

enum intr_status get_intr_status(void);
enum intr_status set_intr_status(enum intr_status status);
enum intr_status intr_enable(void);
enum intr_status intr_disable(void);

void idt_init(void);
void register_handler(uint8_t vector_no, intr_handler function);

#endif
