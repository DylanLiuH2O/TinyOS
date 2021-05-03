[bits 32]
%define ERROR_CODE nop
%define ZERO push 0
extern put_str
extern idt_table

section .data
global intr_entry_table
intr_entry_table:

%macro VECTOR 2
    section .text
      intr%1entry:
      %2
      

      ;保护现场
      push ds
      push es
      push fs
      push gs
      pushad

      mov al,0x20
      out 0xa0,al
      out 0x20,al

      push %1

      call [idt_table + %1*4]   ;去调用C语言编写的中断处理程序
      jmp intr_exit

    section .data
      dd intr%1entry

%endmacro

section .text
global intr_exit
intr_exit:
    add esp,4
    popad       ;恢复现场
    pop gs
    pop fs
    pop es
    pop ds
    add esp,4


    iretd

VECTOR 0x00,ZERO
VECTOR 0x01,ZERO
VECTOR 0x02,ZERO
VECTOR 0x03,ZERO
VECTOR 0x04,ZERO
VECTOR 0x05,ZERO
VECTOR 0x06,ZERO
VECTOR 0x07,ZERO
VECTOR 0x08,ERROR_CODE
VECTOR 0x09,ZERO
VECTOR 0x0a,ERROR_CODE
VECTOR 0x0b,ERROR_CODE
VECTOR 0x0c,ERROR_CODE
VECTOR 0x0d,ERROR_CODE
VECTOR 0x0e,ERROR_CODE
VECTOR 0x0f,ZERO
VECTOR 0x10,ZERO
VECTOR 0x11,ERROR_CODE
VECTOR 0x12,ZERO
VECTOR 0x13,ZERO
VECTOR 0x14,ZERO
VECTOR 0x15,ZERO
VECTOR 0x16,ZERO
VECTOR 0x17,ZERO
VECTOR 0x18,ZERO
VECTOR 0x19,ZERO
VECTOR 0x1a,ZERO
VECTOR 0x1b,ZERO
VECTOR 0x1c,ZERO
VECTOR 0x1d,ZERO
VECTOR 0x1e,ERROR_CODE
VECTOR 0x1f,ZERO
VECTOR 0x20,ZERO        ;时钟中断入口
VECTOR 0x21,ZERO        ;键盘中断入口
VECTOR 0x22,ZERO        ;级联
VECTOR 0x23,ZERO        ;串口2入口
VECTOR 0x24,ZERO        ;串口1入口
VECTOR 0x25,ZERO
VECTOR 0x26,ZERO
VECTOR 0x27,ZERO
VECTOR 0x28,ZERO
VECTOR 0x29,ZERO
VECTOR 0x2a,ZERO
VECTOR 0x2b,ZERO
VECTOR 0x2c,ZERO
VECTOR 0x2d,ZERO
VECTOR 0x2e,ZERO
VECTOR 0x2f,ZERO