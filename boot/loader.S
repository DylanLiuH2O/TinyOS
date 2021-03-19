%include "boot.inc"
SECTION LOADER vstart=LOADER_BASE_ADDR
LOADER_STACK_TOP equ LOADER_BASE_ADDR

    jmp loader_start
;构建GDT及其内部描述符
;注意数据的顺序,x86是小端(低尾端,低字节在低地址上)存储,代码从上到下地址变大
GDT_BASE:         dd 0x00000000         ;低32位
                  dd 0x00000000         ;高32位

CODE_DESC:        dd 0x0000ffff         ;低32位,由于粒度为4KB,且平坦模型下一个段最大寻址范围为4GB,那么段界限值则为4GB/4K=1MB=0x000fffff,这里填低16位
                  dd DESC_CODE_HIGH_4B  ;高32位,直接取boot.inc中定义好的宏

DATA_STACK_DESC:  dd 0x0000ffff         ;同CDOE_DESC
                  dd DESC_DATA_HIGH_4B  ;同CDOE_DESC

VIDEO_DESC:       dd 0x80000007        ;显存段不使用平坦模型,起始地址为0x000b8000,分开三部分存储,0x00,0x0b,0x8000,这里高16位存的第1部分0x8000
                                        ;显存区范围0xb8000~0xbffff,段大小0xbffff-0xb8000=0x7fff,根据粒度4KB可算得段界限0x7fff/4K=0x7,填入这里的低16位
                  dd DESC_VIDEO_HIGH_4B

GDT_SIZE    equ ($ - GDT_BASE) ;$为当前地址,减去GDT_BASE得到GDT大小
GDT_LIMIT   equ GDT_SIZE-1

times 60 dq 0   ;dq是定义描述符数据大小的指令,8个字节,这里预留60个描述符的空位

;定义各段的选择子
SELECTOR_CODE   equ (0x0001 << 3) + TI_GDT + RPL0 ;(0x0001 << 3)是描述符索引值,其值取决于段描述符在GDT中的位置
SELECTOR_DATA   equ (0x0002 << 3) + TI_GDT + RPL0 ;同上
SELECTOR_VIDEO  equ (0x0003 << 3) + TI_GDT + RPL0 ;同上

gdt_ptr dw GDT_LIMIT    ;用于lgdt加载gdt,lgdt 指令格式为lgdt 48位内存数据
        dd GDT_BASE     ;这48位内存数据,低16位GDT以字节为大小的偏移量,高32位为GDT起始地址

loadermsg db '2 loader in real.'



loader_start:

;---------------------------------------------
;INT 0x10 功能号:0x13   使用中断打印字符串
;---------------------------------------------
;input:
;AH 子功能号=13H
;BH = 页码
;BL = 属性
;(DH,DL)=坐标(row,col)
;ES:BP=字符串地址
;AL=显示输出方式
;0 字符串中只含显示字符,显示属性在BL中,显示后光标位置不变
;1 字符串中只含显示字符,显示属性在BL中,显示后光标位置改变
;2 字符串中含显示字符和显示属性,显示后光标位置不变
;3 字符串中含显示字符和显示属性,显示后光标位置改变
;no return

    mov sp,LOADER_BASE_ADDR
    mov bp,loadermsg    ;es:bp 字符串地址
    mov cx,17           ;length of string
    mov ax,0x1301       ;指定功能号和属性
    mov bx,0x001f       ;指定页号(0x00)和颜色(0x1f)
    mov dx,0x1800
    int 0x10            ;中断



;----------- 准备进入保护模式 -------
;step1 打开A20地址线
;step2 加载GDT
;step3  开启CR0的PE位


;step1
    in al,0x92
    or al,0000_0010b
    out 0x92,al


;step2
    lgdt [gdt_ptr]

;step3
    mov eax,cr0
    or eax,0x0000_0001
    mov cr0,eax

    jmp dword SELECTOR_CODE:p_mode_start    ;刷新流水线


[bits 32]
p_mode_start:
    mov ax,SELECTOR_DATA
    mov ds,ax
    mov es,ax
    mov ss,ax
    mov esp,LOADER_STACK_TOP
    mov ax,SELECTOR_VIDEO
    mov gs,ax

    mov byte [gs:160],'p'

    jmp $

