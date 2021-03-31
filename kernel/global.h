#ifndef __KERNEL_GLOBAL_H
#define __KERNEL_GLOBAL_H
#include "stdint.h"

#define RPL0 0
#define RPL1 1
#define RPL2 2
#define RPL3 3

#define TI_GDT 0
#define TI_LDT 1

/*
  15	          3    2  1   0
 -------------------------------
 | descriptor index | TI | RPL |
 -------------------------------
            selector
*/
#define SELECTOR_K_CODE     ((1 << 3) + (TI_GDT << 2) + RPL0)
#define SELECTOR_K_DATA     ((2 << 3) + (TI_GDT << 2) + RPL0)
#define SELECTOR_K_STACK    SELECTOR_K_DATA
#define SELECTOR_K_GS       ((3 << 3) + (TI_GDT << 2) + RPL0)

/*
 31                        16 15 14 13   12   11     8  7   6   5  4         0
 ------------------------------------------------------------------------------
 |  intr_func_offset(31~16)  | P | DPL | S(0) | TYPE() | 0 | 0 | 0 | NOT USED |   High 32-bits
 ------------------------------------------------------------------------------

 31                        16 15                                             0
 ------------------------------------------------------------------------------
 |        func_selector      |             intr_func_offset(15~0)             |   Low 32-bits
 ------------------------------------------------------------------------------

                             Interrupt Gate Descriptor
*/

#define IDT_DESC_P          1
#define IDT_DESC_DPL0       0
#define IDT_DESC_DPL3       3
#define IDT_DESC_32_TYPE    0xe
#define IDT_DESC_16_TYPE    0x6

#define IDT_DESC_ATTR_DPL0 \
    ((IDT_DESC_P << 7) + (IDT_DESC_DPL0 << 5) + IDT_DESC_32_TYPE)
#define IDT_DESC_ATTR_DPL3 \
    ((IDT_DESC_P << 7) + (IDT_DESC_DPL3 << 5) + IDT_DESC_32_TYPE)

#endif
