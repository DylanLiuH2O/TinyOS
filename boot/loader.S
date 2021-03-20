    %include "boot.inc"
    SECTION LOADER vstart=LOADER_BASE_ADDR
    LOADER_STACK_TOP equ LOADER_BASE_ADDR

    ;jmp loader_start    ;跳到开始  去掉跳转,改为在mrb.S中直接跳转到loader_start,跨过数据区,数据区恰好512+256字节,偏移为0x300
    ;构建GDT及其内部描述符
    ;注意数据的顺序,x86是小端(低尾端,低字节在低地址上)存储,代码从上到下地址变大
    GDT_BASE:         dd 0x00000000         ;低32位
                      dd 0x00000000         ;高32位

    CODE_DESC:        dd 0x0000ffff         ;低32位,由于粒度为4KB,且平坦模型下一个段最大寻址范围为4GB,那么段界限值则为4GB/4K=1MB=0x000fffff,这里填低16位
                      dd DESC_CODE_HIGH_4B  ;高32位,直接取boot.inc中定义好的宏

    DATA_STACK_DESC:  dd 0x0000ffff         ;同CDOE_DESC
                      dd DESC_DATA_HIGH_4B  ;同CDOE_DESC

    VIDEO_DESC:       dd 0x80000007         ;显存段不使用平坦模型,起始地址为0x000b8000,分开三部分存储,0x00,0x0b,0x8000,这里高16位存的第1部分0x8000
                                            ;显存区范围0xb8000~0xbffff,段大小0xbffff-0xb8000=0x7fff,根据粒度4KB可算得段界限0x7fff/4K=0x7,填入这里的低16位
                      dd DESC_VIDEO_HIGH_4B

    GDT_SIZE    equ ($ - GDT_BASE)          ;$为当前地址,减去GDT_BASE得到GDT大小
    GDT_LIMIT   equ GDT_SIZE-1

    times 60 dq 0                           ;double quad word,8个字节,这里预留60个描述符的空位

    total_mem_bytes dd 0                    ;用于保存内存容量,单位是字节
                                            ;这里相对于文件头的偏移地址为0x200,前面刚好512字节
                                            ;而loader加载到0x900,因此这里实际的物理地址为0x900+0x200=0xb00

    ;定义各段的选择子
    SELECTOR_CODE   equ (0x0001 << 3) + TI_GDT + RPL0   ;(0x0001 << 3)是描述符索引值,其值取决于段描述符在GDT中的位置
    SELECTOR_DATA   equ (0x0002 << 3) + TI_GDT + RPL0   ;同上
    SELECTOR_VIDEO  equ (0x0003 << 3) + TI_GDT + RPL0   ;同上

    gdt_ptr dw GDT_LIMIT                                ;用于lgdt加载gdt,lgdt 指令格式为lgdt 48位内存数据
            dd GDT_BASE                                 ;这48位内存数据,低16位GDT以字节为大小的偏移量,高32位为GDT起始地址

    loadermsg db '2 loader in real.'

    ards_buf times 244 db 0                             ;total_mem_bytes(4bytes) + gdt_ptr(6bytes) + ards_buf(244bytes) + ard_nr(2bytes) = 256bytes,对齐
    ards_num dw 0                                       ;记录ards结构体数量

    loader_start:

    ;---------------- 使用三种方法检测内存容量 -----------------------
    ;---------------- 方法1:使用中断0x15子功能0xe820h获取内存布局 ----
    mov edx,0x534d4150          ;SMAP的ASCII码作为签名
    xor ebx,ebx                 ;清零
    mov di,ards_buf             ;ards_buf缓冲区地址
.e820_get_mem_loop:
    mov eax,0x0000e820          ;每次执行中断后,eax的值会变成签名的值,所以要每次都更改回来
    mov ecx,20                  ;ards结构字节的大小,指示BIOS写入的字节数
    int 0x15
    jc .e820_failed_jmp_to_e801  ;检测CF位,如果为1则调用出错,跳转到下一个方法执行
    add di,cx
    inc word [ards_num]         ;记录数量
    cmp ebx,0                   ;判断这是否最后一个ards结构
    jnz .e820_get_mem_loop


    mov cx,[ards_num]
    mov ebx,ards_buf
    xor edx,edx                 ;用edx来存储最大值
.find_max_mem_area:
    mov eax,[ebx]
    add eax,[ebx+8]
    add ebx,20
    cmp edx,eax
    jge .next_ards
    mov edx,eax
    .next_ards:
    loop .find_max_mem_area

    ;---------------- 方法2:使用中断0x15子功能0xe810h获取内存信息 ----
.e820_failed_jmp_to_e801:
    mov ax,0xe801
    int 0x15
    jc .e801_failed_jmp_to_88

    mov cx,0x400        ;单位1KB
    mul cx

    ;获得完整的32位积
    shl edx,16
    and eax,0x0000ffff
    or edx,eax

    add edx,0x100000    ;加上预留的1MB   
    mov esi,edx

    xor eax,eax
    mov ax,bx
    mov ecx,0x10000      ;单位64KB
    mul ecx

    add esi,eax
    mov edx,esi
    jmp .mem_get_ok

    ;---------------- 方法3:使用中断0x15子功能0x88h获取内存信息 ----
    ;-----------------------------------------------------------------
.e801_failed_jmp_to_88:
    mov ah,0x88
    int 0x15
    jc .error_hlt
    and eax,0x0000ffff

    mov cx,0x400
    mul cx
    shl edx,16
    or edx,eax
    add edx,0x10000

.mem_get_ok:
    mov [total_mem_bytes],edx   

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

    ;出错则挂起 
.error_hlt:
    hlt

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

