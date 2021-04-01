#ifndef __INTERRUPT_H
#define __INTERRUPT_H

typedef void* intr_handler;

void idt_init(void);

enum intr_status {
    INTR_OFF,       //关中断
    INTR_ON         //开中断
};

enum intr_status get_intr_status(void);
enum intr_status set_intr_status(enum intr_status status);
enum intr_status intr_enable(void);
enum intr_status intr_disable(void);

#endif
